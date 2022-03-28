In last lesson we've used `TRUE` constant everywhere.

Here are other constants with respect to their IFR code that can be used in VFR:
```
"TRUE"		EFI_IFR_TRUE 
"FALSE"		EFI_IFR_FALSE
"ONE"		EFI_IFR_ONE 		 (=1)
"ONES"		EFI_IFR_ONES		 (=0xFFFFFFFFFFFFFFFF)
"ZERO"		EFI_IFR_ZERO             (=0)
"UNDEFINED"	EFI_IFR_UNDEFINED	 (=Undefined)
"VERSION"	EFI_IFR_VERSION 	 (=UEFI specification to which the Forms Processor conforms)
```

Off couse there is also a number constant. It can be one of the folowing types:
```
EFI_IFR_UINT8
EFI_IFR_UINT16
EFI_IFR_UINT32
EFI_IFR_UINT64
```

Several operations are supported on numbers:
```
"+"		EFI_IFR_ADD
"-"		EFI_IFR_SUBTRACT
"*"		EFI_IFR_MULTIPLY
"/"		EFI_IFR_DIVIDE
"%"		EFI_IFR_MODULO
"|"		EFI_IFR_BITWISE_OR
"&"		EFI_IFR_BITWISE_AND
"=="		EFI_IFR_EQUAL
"!="		EFI_IFR_NOT_EQUAL
"<"		EFI_IFR_LESS_THAN
"<="		EFI_IFR_LESS_EQUAL
">"		EFI_IFR_GREATER_THAN
">="		EFI_IFR_IFR_GREATER_EQUAL
"<<"		EFI_IFR_SHIFT_LEFT
">>"		EFI_IFR_SHIFT_RIGHT
"~"		EFI_IFR_BITWISENOT
```

Here are logical operators:
```
"OR"		EFI_IFR_OR
"AND"		EFI_IFR_AND
"NOT"           EFI_IFR_NOT
```

Ternary operator (short form for `if-then-else`, i.e. `<...> ? <...> : <...>`) is also supported in VFR in a form:
```
cond( <...> ? <...> : <...> )		EFI_IFR_CONDITIONAL
```

# casts

Keep in mind that although something simple like this might work:
```
suppressif 0;
  ...
endif;
```
When you treat numbers as booleans, you should explicitly convert them to bool. Else it can be the source of hard to find errors. For example:
```
suppressif (1 OR 0);                    // element is present (incorrect)
  ...
endif;

suppressif ((BOOLEAN)1 OR (BOOLEAN)0);  // element is not present (correct)
  ...
endif;
```

Here are complete list of casts that you can use in code:
```
"(BOOLEAN)"	EFI_IFR_TO_BOOLEAN
"(UINT64)"	EFI_IFR_TO_UINT
"(UINT32)"
"(UINT16)"
"(UINT8)"
```

Besides these casts you can use:
```
boolval(<...>)		EFI_IFR_TO_BOOLEAN
stringval(<...>)	EFI_IFR_TO_STRING
unintval(<...>)		EFI_IFR_TO_UINT
```

# ideqval

If you want to compare question value to UINT16 value, you can use `ideqval` syntax (this is an abbreviation for the "id equal value"):

```
EFI_IFR_EQ_ID_VAL

Summary:
Push TRUE if a question’s value is equal to a 16-bit unsigned integer, otherwise FALSE.

Prototype:

#define EFI_IFR_EQ_ID_VAL_OP 0x12

typedef struct _EFI_IFR_EQ_ID_VAL {
 EFI_IFR_OP_HEADER Header;
 EFI_QUESTION_ID QuestionId;
 UINT16 Value;
} EFI_IFR_EQ_ID_VAL;

Members:
Header 		Standard opcode header, where OpCode is EFI_IFR_EQ_ID_VAL_OP.
QuestionId 	Specifies the identifier of the question whose value will be compared.
Value 		Unsigned integer value to compare against.

Description:
Evaluate the value of the specified question (QuestionId). If the specified question cannot be evaluated as an unsigned integer, then push Undefined. If they are equal, push TRUE. Otherwise push FALSE.
```

For example the following expressions are allowed:
```
suppressif ideqval FormData.CheckboxValue == 1;
  ...
endif;

suppressif ideqval FormData.NumericValue == 7;
  ...
endif

suppressif ideqval FormData.OneOfValue == 0x33;
  ...
endif;
```

This means that form browser will dynamically hide/show element, based on the value of another element.

Keep in mind that you shouldn't put anything in place of `==` as it would be against language syntax and can lead to undefined behaviour. If you won't to test if the value is unequal, you can utilize `NOT` keyword:

```
suppressif NOT ideqval FormData.OneOfValue == 0x33;
  ...
endif;
```

# ideqvallist

The `ideqvallist` (this is an abbreviation for the "id equal value list") element compares question value agains a list of `UINT16` values:

```
EFI_IFR_EQ_ID_VAL_LIST

Summary:
Push TRUE if the question’s value appears in a list of unsigned integers.

Prototype:

#define EFI_IFR_EQ_ID_VAL_LIST_OP 0x14

typedef struct _EFI_IFR_EQ_ID_VAL_LIST {
 EFI_IFR_OP_HEADER Header;
 EFI_QUESTION_ID QuestionId;
 UINT16 ListLength;
 UINT16 ValueList[1];		// Practically this is `ValueList[ListLength]`
} EFI_IFR_EQ_ID_VAL_LIST;

Members:
Header		Standard opcode header, where OpCode is EFI_IFR_EQ_ID_VAL_LIST_OP.
QuestionId	Specifies the identifier of the question whose value will be compared.
ListLength 	Number of entries in ValueList.
ValueList 	Zero or more unsigned integer values to compare against.

Description:
Evaluate the value of the specified question (QuestionId). If the specified question cannot be evaluated as an unsigned integer, then push Undefined. If the value can be found in ValueList, then push TRUE. Otherwise push FALSE.
```


For example the following expressions are allowed:
```
suppressif ideqvallist FormData.NumericValue == 7 9 10 15;
  ...
endif

suppressif ideqvallist FormData.OneOfValue == 0x33 0x55;
  ...
endif;
```

# ideqid

The `ideqid` (this is an abbreviation for the "id equal id") builtin can compare values of two questions on the form:
```
EFI_IFR_EQ_ID_ID

Summary:
Push TRUE if the two questions have the same value or FALSE if they are not equal.

Prototype:

#define EFI_IFR_EQ_ID_ID_OP 0x13

typedef struct _EFI_IFR_EQ_ID_ID {
 EFI_IFR_OP_HEADER Header;
 EFI_QUESTION_ID QuestionId1;
 EFI_QUESTION_ID QuestionId2;
} EFI_IFR_EQ_ID_ID;

Members:
Header				Standard opcode header, where OpCode is EFI_IFR_EQ_ID_ID_OP.
QuestionId1, QuestionId2	Specifies the identifier of the questions whose values will be compared.

Description:
Evaluate the values of the specified questions (QuestionId1, QuestionId2). If the two values cannot be evaluated or cannot be converted to comparable types, then push Undefined. If they are equal, push TRUE. Otherwise push FALSE.
```

Example:
```
suppressif ideqid FormData.OneOfValue == FormData.NumericValue
  ...
endif;
```

# questionref

The statements above (`ideqval`/`ideqvallist`/`ideqid`) allow tests only for equality. If you want more complex math you can use `questionref`:
```
EFI_IFR_QUESTION_REF1

Summary:
Push a question’s value on the expression stack.

Prototype:

#define EFI_IFR_QUESTION_REF1_OP 0x40

typedef struct _EFI_IFR_QUESTION_REF1 {
 EFI_IFR_OP_HEADER Header;
 EFI_QUESTION_ID QuestionId;
} EFI_IFR_QUESTION_REF1;

Members:
Header 		The byte sequence that defines the type of opcode as well as the length of the opcode being defined.
		Header.OpCode = EFI_IFR_QUESTION_REF1_OP.
QuestionId 	The question’s identifier, which must be unique within the form set.

Description:
Push the value of the question specified by QuestionId on to the expression stack. If the question’s value cannot be determined or the question does not exist, then push Undefined.
```

One important change here is that the question reference goes not by `varid` (like `FormData.NumericValue`), but by a value of the `name` field that you need to declare in the question:
```
numeric
  name = NumericQuestion,
  ...
endnumeric;

suppressif (BOOLEAN)(questionref(NumericQuestion) % 2);
  ...
endif
```
This code will show element, only if numeric value is even.

Keep in mind that code defining a `name` must come before its reference in `questionref`. Opposite situation would lead to build failure.

# pushthis

Inside the question you can get the question value with the help of `pushthis` keyword, which would translate to `EFI_IFR_THIS`:
```
EFI_IFR_THIS

Summary:
Push current question’s value.

Prototype:

#define EFI_IFR_THIS_OP 0x58

typedef struct _EFI_IFR_THIS {
 EFI_IFR_OP_HEADER Header;
} EFI_IFR_THIS;

Members:
Header 		The sequence that defines the type of opcode as well as the length of the opcode being defined.
		For this tag, Header.OpCode = EFI_IFR_THIS_OP.

Description:
Push the current question’s value.
```

For example:
```
numeric
  name = NumericQuestion,
  varid = FormData.NumericValue,
  prompt = STRING_TOKEN(NUMERIC_PROMPT),
  help = STRING_TOKEN(NUMERIC_HELP),
  minimum = 5,
  maximum = 20,
  warningif
    prompt = STRING_TOKEN(WARNING_IF_PROMPT),
    pushthis == 10
  endif;
endnumeric;
```

In this case you can achive similar result with `ideqval` or `questionref`. These statements would be equivalent:
```
ideqval FormData.NumericValue == 10

questionref(NumericQuestion) == 10

pushthis == 10
```

Another example:
```
string
  varid = FormData.StringValue,
  prompt = STRING_TOKEN(STRING_PROMPT),
  help = STRING_TOKEN(STRING_HELP),
  minsize = 5,
  maxsize = 10,
  warningif
    prompt = STRING_TOKEN(WARNING_IF_PROMPT),
    pushthis == stringref(STRING_TOKEN(TEST_STRING))
  endif;
endstring;
```
Here warning window will appear if user inputs "EDKII" to the string form element.
