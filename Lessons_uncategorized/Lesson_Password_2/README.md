In the last lesson we've investigated the VFR `password` element and developed a code that was saving the password value as a plain text in the `varstore` storage. But in reality you shouldn't store passwords as it is in your storage! This is a security vulnerability. Instead you should at least store a hash value of the password. So even if the database is compromised, the attacker can't reverse the original password value.

Here is a simple example. If our form has only the `password element` it wouldn't be present in the `HIIConfig dump` output. But when I've added a `checkbox` element to the form, the `HIIConfig dump` printed all the values inside the `varstorage` including the current value for the password field:
```
GUID=3cd7f2e79a69064692b6a35e4927c4d4 (E7F2D73C-699A-4606-92B6-A35E4927C4D4)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=010414009fcf89b211f9fd419bad2c92576886647fff0400 (VenHw(B289CF9F-F911-41FD-9BAD-2C9257688664))
OFFSET=0  WIDTH=0000000000000013  VALUE=0100000074006500720063006500730079006d
6D 00 79 00 73 00 65 00 63 00 72 00 65 00 74 00  | m.y.s.e.c.r.e.t.
00 00 01                                         | ...

GUID=3cd7f2e79a69064692b6a35e4927c4d4 (E7F2D73C-699A-4606-92B6-A35E4927C4D4)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=010414009fcf89b211f9fd419bad2c92576886647fff0400 (VenHw(B289CF9F-F911-41FD-9BAD-2C9257688664))
ALTCFG=0000
OFFSET=0012  WIDTH=0001  VALUE=01
01                                               | .

GUID=3cd7f2e79a69064692b6a35e4927c4d4 (E7F2D73C-699A-4606-92B6-A35E4927C4D4)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=010414009fcf89b211f9fd419bad2c92576886647fff0400 (VenHw(B289CF9F-F911-41FD-9BAD-2C9257688664))
ALTCFG=0001
OFFSET=0012  WIDTH=0001  VALUE=01
01                                               | .
```

This is clearly no good! This is a potential loophole for the attaker to get our password. Not sure if this is a vulnerability in the edk2 code, but at least we can protect the initial password by storing it not as a simple string, but as something like SHA512 hash value of the password string.

Let's investigate how we can do this.

For the hash calculation UEFI specification defines the `EFI_HASH2_PROTOCOL` protocol. Before diving into its specification, let's understand how to get it.

This protocol is not supposed to be acquired via the usual `LocateProtocol` routine. It is supposed to be acquired via the standard EFI service binding scheme. To get the `EFI_HASH2_PROTOCOL` you need to call `CreateChild()` on the `EFI_HASH2_SERVICE_BINDING_PROTOCOL` and perform `HandleProtocol()` on the returned handle. To clear the resources on driver unload we need to call the `DestroyChild` respectively.

With the service binding scheme multiple drivers can use the hashing services. Or even one driver can have multiple simultaneous non-overlapping hash operations.

So let's try to get the `EFI_HASH2_PROTOCOL` protocol:
```cpp
<...>

#include <Protocol/Hash2.h>
#include <Protocol/ServiceBinding.h>

<...>

EFI_SERVICE_BINDING_PROTOCOL* hash2ServiceBinding;
EFI_HASH2_PROTOCOL* hash2Protocol;
EFI_HANDLE hash2ChildHandle = NULL;

<...>

EFI_STATUS
EFIAPI
PasswordFormWithHashUnload (
  EFI_HANDLE ImageHandle
  )
{
  hash2ServiceBinding->DestroyChild(hash2ServiceBinding, hash2ChildHandle);

  <...>
}

EFI_STATUS
EFIAPI
PasswordFormWithHashEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;
  Status = gBS->LocateProtocol(&gEfiHash2ServiceBindingProtocolGuid,
                               NULL,
                               (VOID **)&hash2ServiceBinding);
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't locate gEfiHash2ServiceBindingProtocolGuid: %r\n", Status);
    return Status;
  }

  Status = hash2ServiceBinding->CreateChild(hash2ServiceBinding,
                                            &hash2ChildHandle);
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't create child on gEfiHash2ServiceBindingProtocolGuid: %r\n", Status);
    return Status;
  }

  Status = gBS->OpenProtocol(hash2ChildHandle,
                             &gEfiHash2ProtocolGuid,
                             (VOID **)&hash2Protocol,
                             NULL,
                             hash2ChildHandle,
                             EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't open gEfiHashProtocolGuid: %r\n", Status);
    return Status;
  }

  <...>
}
```

Don't forget to add the necessary protocols to the INF file:
```
[Protocols]
  ...
  gEfiHash2ServiceBindingProtocolGuid
  gEfiHash2ProtocolGuid
```

Now to the actual `EFI_HASH2_PROTOCOL` itself:
```
EFI_HASH2_PROTOCOL

Summary:
This protocol describes hashing functions for which the algorithm-required message padding and finalization are performed by the supporting driver. In previous versions of the specification, the algorithms supported by EFI_HASH2_PROTOCOL were also available for use with EFI_HASH_PROTOCOL but this usage has been deprecated.

GUID:
#define EFI_HASH2_PROTOCOL_GUID { 0x55b1d734, 0xc5e1, 0x49db, 0x96, 0x47, 0xb1, 0x6a, 0xfb, 0xe, 0x30, 0x5b}

typedef _EFI_HASH2_PROTOCOL {
 EFI_HASH2_GET_HASH_SIZE GetHashSize;
 EFI_HASH2_HASH Hash;
 EFI_HASH2_HASH_INIT HashInit;
 EFI_HASH2_HASH_UPDATE HashUpdate;
 EFI_HASH2_HASH_FINAL HashFinal;
} EFI_HASH2_PROTOCOL;

Parameters:
GetHashSize  Return the result size of a specific type of resulting hash.
Hash         Create a final non-extendable hash for a single message block in a single call.
HashInit     Initializes an extendable multi-part hash calculation
HashUpdate   Continues a hash in progress by supplying the first or next sequential portion of the message text
HashFinal    Finalizes a hash in progress by padding as required by algorithm and returning the hash output.
```

In the edk2 this protocol is defined in the header [MdePkg/Include/Protocol/Hash2.h](file https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/Hash2.h).

In our simple case we would use the `EFI_HASH2_HASH Hash` function.

If you want to know about the `HashInit`/`HashUpdate`/`HashFinal` functions, I think it is better to illustrate their work on a simple example. The 3 cases below would give you identical hash results:
```
Hash("ABCDEF") | HashInit()           | HashInit ()
               | HashUpdate("ABCDEF") | HashUpdate ("ABC")
               | HashFinal()          | HashUpdate ("DE")
               |                      | HashUpdate ("F")
               |                      | HashFinal ()
```
I hope this is pretty self-explanatory.

Now let's see the specification for the `EFI_HASH2_HASH Hash` function that we would use:
```
EFI_HASH2_PROTOCOL.Hash()

Summary:
Creates a hash for a single message text. The hash is not extendable. The output is final with any algorithm-required
padding added by the function.

Prototype:

EFI_STATUS
EFIAPI
Hash(
  IN CONST EFI_HASH2_PROTOCOL *This,
  IN CONST EFI_GUID *HashAlgorithm,
  IN CONST UINT8 *Message,
  IN UINTN MessageSize,
  IN OUT EFI_HASH2_OUTPUT *Hash
);

Parameters:
This           Points to this instance of EFI_HASH2_PROTOCOL
HashAlgorithm  Points to the EFI_GUID which identifies the algorithm to use
Message        Points to the start of the message
MessageSize    The size of Message, in bytes
Hash           On input, points to a caller-allocated buffer of the size returned by GetHashSize() for the specified HashAlgorithm
               On output, the buffer holds the resulting hash computed from the message

Description:
This function creates the hash of specified single block message text based on the specified algorithm HashAlgorithm and copies the result to the caller-provided buffer Hash. The resulting hash cannot be extended. All padding required by HashAlgorithm is added by the implementation.
```

The possible hash algorithms and their respected GUIDs are present below:
```
SHA-1:  EFI_HASH_ALGORITHM_SHA1_GUID
SHA224: EFI_HASH_ALGORITHM_SHA224_GUID
SHA256: EFI_HASH_ALGORITHM_SHA256_GUID
SHA384: EFI_HASH_ALGORITHM_SHA384_GUID
SHA512: EFI_HASH_ALGORITHM_SHA512_GUID
MD5:    EFI_HASH_ALGORTIHM_MD5_GUID
```

We would use the `EFI_HASH_ALGORITHM_SHA512_GUID` as the strongest of the above in our hash calculation.

The `EFI_HASH2_PROTOCOL.Hash()` returns its output in the `EFI_HASH2_OUTPUT *Hash` argument. The `EFI_HASH2_OUTPUT` type is a `enum` defined like this:
```cpp
typedef UINT8 EFI_MD5_HASH2[16];
typedef UINT8 EFI_SHA1_HASH2[20];
typedef UINT8 EFI_SHA224_HASH2[28];
typedef UINT8 EFI_SHA256_HASH2[32];
typedef UINT8 EFI_SHA384_HASH2[48];
typedef UINT8 EFI_SHA512_HASH2[64];

typedef union {
  EFI_MD5_HASH2       Md5Hash;
  EFI_SHA1_HASH2      Sha1Hash;
  EFI_SHA224_HASH2    Sha224Hash;
  EFI_SHA256_HASH2    Sha256Hash;
  EFI_SHA384_HASH2    Sha384Hash;
  EFI_SHA512_HASH2    Sha512Hash;
} EFI_HASH2_OUTPUT;
```

In our form storage we need to create appropriate field to store the password hash. Since we need to know the size of the storage at compile time and our hash algorithm is fixed, instead of using the protocol `EFI_HASH2_GET_HASH_SIZE GetHashSize` we would just define it as a static number. From the description above you can see that the size of the SHA512 hash is 64:
```
#define HASHED_PASSWORD_SIZE   64

#pragma pack(1)
typedef struct {
  UINT8 Password[HASHED_PASSWORD_SIZE];
} VARIABLE_STRUCTURE;
#pragma pack()
```

Here you can also observe that now `Password` field is not even a `CHAR16` string, but a `UINT8 []`.

Now we are ready to create a `ComputeStringHash` function that would calculate a hash from the `EFI_STRING`:
```cpp
EFI_STATUS ComputeStringHash(EFI_STRING Password, UINT8* HashedPassword)
{
  EFI_GUID HashGuid = EFI_HASH_ALGORITHM_SHA512_GUID;
  EFI_HASH2_OUTPUT Hash;
  EFI_STATUS Status = hash2Protocol->Hash(hash2Protocol,
                                          &HashGuid,
                                          (UINT8*)Password,
                                          StrLen(Password)*sizeof(CHAR16),
                                          &Hash);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  CopyMem(HashedPassword, Hash.Sha512Hash, HASHED_PASSWORD_SIZE);
  return EFI_SUCCESS;
}
```

With it we can update our `HandlePasswordInput` function to store hashes instead of plain passwords:
```cpp
EFI_STATUS HandlePasswordInput(EFI_STRING Password)
{
  EFI_STATUS Status;

  if (Password[0] == 0) {
    // Form Browser checks if password exists
    if (FormStorage.Password[0] != 0) {
      return EFI_ALREADY_STARTED;
    } else {
      return EFI_SUCCESS;
    }
  } else {
    // Form Browser sends password value
    // It can be old password to check or initial/updated password to set

    if (FormStorage.Password[0] == 0) {
      // Set initial password
      ComputeStringHash(Password, FormStorage.Password);
      if (EFI_ERROR(Status)) {
        return Status;
      }
      return EFI_SUCCESS;
    }

    if (!OldPasswordVerified) {
      // Check old password
      UINT8 TempHash[HASHED_PASSWORD_SIZE];
      ComputeStringHash(Password, TempHash);
      if (CompareMem(TempHash, FormStorage.Password, HASHED_PASSWORD_SIZE))
        return EFI_NOT_READY;

      OldPasswordVerified = TRUE;
      return EFI_SUCCESS;
    }

    // Update password
    Status = ComputeStringHash(Password, FormStorage.Password);
    if (EFI_ERROR(Status)) {
      return Status;
    }
    OldPasswordVerified = FALSE;
    return EFI_SUCCESS;
  }
}
```

Now we are ready to build and test our driver.

Unfortunately the driver would fail on load:
```
FS0:\> load PasswordFormWithHash.efi
Error! Can't locate gEfiHash2ServiceBindingProtocolGuid: Not Found
Image 'FS0:\PasswordFormWithHash.efi' error in StartImage: Not Found
```

As you can see there is no `EFI_HASH2_SERVICE_BINDING_PROTOCOL` in the default OVMF build. So let's try to fix it.

The `EFI_HASH2_SERVICE_BINDING_PROTOCOL` is produced by the [Hash2DxeCrypto](https://github.com/tianocore/edk2/tree/master/SecurityPkg/Hash2DxeCrypto).

Since its module type `UEFI_DRIVER` we can just build it:
```
build --platform=SecurityPkg/SecurityPkg.dsc --module=SecurityPkg/Hash2DxeCrypto/Hash2DxeCrypto.inf --arch=X64 --buildtarget=RELEASE --tagname=GCC5
cp Build/SecurityPkg/RELEASE_GCC5/X64/SecurityPkg/Hash2DxeCrypto/Hash2DxeCrypto/OUTPUT/Hash2DxeCrypto.efi ~/disk
```

and load in shell:
```
FS0:\> load Hash2DxeCrypto.efi
Image 'FS0:\Hash2DxeCrypto.efi' loaded at 640D000 - Success
```

Now our driver `PasswordFormWithHash` can load without any problems
```
FS0:\> load PasswordFormWithHash.efi
Image 'FS0:\PasswordFormWithHash.efi' loaded at 63D8000 - Success
```

You can test that the form works with the password element the same as before.

The difference is not visible to the user, but now if we repeat the exploit presented at the start of this lesson, this is what we would get:
```
GUID=3cd7f2e79a69064692b6a35e4927c4d4 (E7F2D73C-699A-4606-92B6-A35E4927C4D4)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=010414009fcf89b211f9fd419bad2c92576886647fff0400 (VenHw(B289CF9F-F911-41FD-9BAD-2C9257688664))
OFFSET=0  WIDTH=0000000000000041  VALUE=01a71c52e40993cb53ae8fc16c3ad3c0596cc03b67a225bef0effa0f43e9f362921ce17b26f9cf7ad6ed487fa955438206d0f211bffea4e9d216eb2b0d2133882f
2F 88 33 21 0D 2B EB 16 D2 E9 A4 FE BF 11 F2 D0  | /.3!.+..........
06 82 43 55 A9 7F 48 ED D6 7A CF F9 26 7B E1 1C  | ..CU..H..z..&{..
92 62 F3 E9 43 0F FA EF F0 BE 25 A2 67 3B C0 6C  | .b..C.....%.g;.l
59 C0 D3 3A 6C C1 8F AE 53 CB 93 09 E4 52 1C A7  | Y..:l...S....R..
01                                               | .

GUID=3cd7f2e79a69064692b6a35e4927c4d4 (E7F2D73C-699A-4606-92B6-A35E4927C4D4)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=010414009fcf89b211f9fd419bad2c92576886647fff0400 (VenHw(B289CF9F-F911-41FD-9BAD-2C9257688664))
ALTCFG=0000
OFFSET=0040  WIDTH=0001  VALUE=01
01                                               | .

GUID=3cd7f2e79a69064692b6a35e4927c4d4 (E7F2D73C-699A-4606-92B6-A35E4927C4D4)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=010414009fcf89b211f9fd419bad2c92576886647fff0400 (VenHw(B289CF9F-F911-41FD-9BAD-2C9257688664))
ALTCFG=0001
OFFSET=0040  WIDTH=0001  VALUE=01
01                                               | .
```

As you can see now the form storage contains not a password itself, but a hash value of the password. This way even if the storage is compromised the potential attaker still wouldn't have the original password value.


# Compiling `Hash2DxeCrypto` to the OVMF

As you saw earlier by default the OVMF doesn't populate the `EFI_HASH2_SERVICE_BINDING_PROTOCOL`. To get it we needed to manually load the `Hash2DxeCrypto.efi` driver.

Alternatively we can include the `Hash2DxeCrypto` to the OVMF build, so it would execute in the DXE stage every time on the machine load.

For that we need to add it to the DSC (`OvmfPkg/OvmfPkgX64.dsc`):
```
[Components]

<...>

SecurityPkg/Hash2DxeCrypto/Hash2DxeCrypto.inf
```

And to the FDF (`OvmfPkg/OvmfPkgX64.fdf`):
```
[FV.DXEFV]

<...>

INF SecurityPkg/Hash2DxeCrypto/Hash2DxeCrypto.inf
```

And recompile the OVMF image after the changes:
```
build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=RELEASE --tagname=GCC5
```

Now you should be able to load our `PasswordFormWithHash` application right after the shell load.

# Alternatives to the `EFI_HASH2_PROTOCOL`

Instead of using the `EFI_HASH2_PROTOCOL` you can just use the hash functions from the `BaseCryptLib` library [https://github.com/tianocore/edk2/blob/master/CryptoPkg/Include/Library/BaseCryptLib.h](https://github.com/tianocore/edk2/blob/master/CryptoPkg/Include/Library/BaseCryptLib.h).

For example here is a description for the `Sha512HashAll` function:
```cpp
/**
  Computes the SHA-512 message digest of a input data buffer.

  This function performs the SHA-512 message digest of a given data buffer, and places
  the digest value into the specified memory.

  If this interface is not supported, then return FALSE.

  @param[in]   Data        Pointer to the buffer containing the data to be hashed.
  @param[in]   DataSize    Size of Data buffer in bytes.
  @param[out]  HashValue   Pointer to a buffer that receives the SHA-512 digest
                           value (64 bytes).

  @retval TRUE   SHA-512 digest computation succeeded.
  @retval FALSE  SHA-512 digest computation failed.
  @retval FALSE  This interface is not supported.

**/
BOOLEAN
EFIAPI
Sha512HashAll (
  IN   CONST VOID  *Data,
  IN   UINTN       DataSize,
  OUT  UINT8       *HashValue
  );
```

We can just use it in our calculations simple as this:
```
#include <Library/BaseCryptLib.h>

...

UINT8 Hash[SHA512_DIGEST_SIZE];
BOOLEAN Status = Sha512HashAll(Password, StrLen(Password) * sizeof(CHAR16), Hash);
if (!Status) {
  return EFI_DEVICE_ERROR;
}
return EFI_SUCCESS;
```

The downside of this approach is that this will definitely blow the driver image size. When we use the function from the protocol, in the nutshell we just find some memory address and cast it to a target function. So no actual impementation code of the function is present in our driver.

On the other case when you use the library approach, all the function implementation code would be encoded in our driver at compile-time. This would bloat the image, but will make the driver self-sufficient.

Right now edk2 is in the process of refactoring code to the usage of `EFI_HASH2_PROTOCOL`. Most of the code that you would find now is based on the library approach. But nevertheless a more correct approach is to move towards the protocol solution.
