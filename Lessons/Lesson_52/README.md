UNI files is intendend for the localization in UEFI. Up until now we've seen only English and French languages in the UNI files. Both of these languages are common in a sense that they both use symbols from the latin charachter set.

If you'll try to print string from another character set you woudn't see the expected output. For example simple "Hello!" in Russian
```
Print(L"Привет!\n");
```
would be printed like this:

QEMU would print something like this in `nographic` mode:
```
FS0:\> HIIFont.efi
?@825B!
```
Don't look at this output, this is happening from another translation level, in this lesson look at the UEFI graphic (either native or from the vnc).

Anyway as you can see only the `!` was printed from the whole string. This happend because there is no russian font in our UEFI system. It simply doesn't know how to transform russian unicode symbol codes to their symbol images.

Font information is stored in the HII Database, therefore to fix the issue we simply need to provide a Package list with a package of Font type with "pictures" for unicode symbols with russian codes.

UEFI uses 8x19 font for narrow symbols and 16x19 font for wide symbols. In the code data for symbols is encoded in structures `EFI_NARROW_GLYPH` and `EFI_WIDE_GLYPH` https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Uefi/UefiInternalFormRepresentation.h:
```
#define EFI_GLYPH_HEIGHT                     19
#define EFI_GLYPH_WIDTH                      8
///@}

///
/// The EFI_NARROW_GLYPH has a preferred dimension (w x h) of 8 x 19 pixels.
///
typedef struct {
  ///
  /// The Unicode representation of the glyph. The term weight is the
  /// technical term for a character code.
  ///
  CHAR16                 UnicodeWeight;
  ///
  /// The data element containing the glyph definitions.
  ///
  UINT8                  Attributes;
  ///
  /// The column major glyph representation of the character. Bits
  /// with values of one indicate that the corresponding pixel is to be
  /// on when normally displayed; those with zero are off.
  ///
  UINT8                  GlyphCol1[EFI_GLYPH_HEIGHT];
} EFI_NARROW_GLYPH;

///
/// The EFI_WIDE_GLYPH has a preferred dimension (w x h) of 16 x 19 pixels, which is large enough
/// to accommodate logographic characters.
///
typedef struct {
  ///
  /// The Unicode representation of the glyph. The term weight is the
  /// technical term for a character code.
  ///
  CHAR16                 UnicodeWeight;
  ///
  /// The data element containing the glyph definitions.
  ///
  UINT8                  Attributes;
  ///
  /// The column major glyph representation of the character. Bits
  /// with values of one indicate that the corresponding pixel is to be
  /// on when normally displayed; those with zero are off.
  ///
  UINT8                  GlyphCol1[EFI_GLYPH_HEIGHT];
  ///
  /// The column major glyph representation of the character. Bits
  /// with values of one indicate that the corresponding pixel is to be
  /// on when normally displayed; those with zero are off.
  ///
  UINT8                  GlyphCol2[EFI_GLYPH_HEIGHT];
  ///
  /// Ensures that sizeof (EFI_WIDE_GLYPH) is twice the
  /// sizeof (EFI_NARROW_GLYPH). The contents of Pad must
  /// be zero.
  ///
  UINT8                  Pad[3];
} EFI_WIDE_GLYPH;
```

I honestly don't know why, but you can find examples for some hebrew letters in the https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Application/UiApp/String.c in the `mFontBin` structure. Let's examine one symbol from this structure - symbol with a unicode code `0x05d2`. Look at the `GlyphCol1` array and try to print array data in binary system. It is kinda hard to see an image in a `1/0` picture, so here I've provided `X/-` picture as well:
```
    {
      0x05d2,
      0x00,
      {
        0x00,      // 00000000 // --------
        0x00,      // 00000000 // --------
        0x00,      // 00000000 // --------
        0x78,      // 01111000 // -XXXX---
        0x7C,      // 01111100 // -XXXXX--
        0x0C,      // 00001100 // ----XX--
        0x0C,      // 00001100 // ----XX--
        0x0C,      // 00001100 // ----XX--
        0x0C,      // 00001100 // ----XX--
        0x0C,      // 00001100 // ----XX--
        0x0C,      // 00001100 // ----XX--
        0x1C,      // 00011100 // ---XXX--
        0x3E,      // 00111110 // --XXXXX-
        0x66,      // 01100110 // -XX--XX-
        0x66,      // 01100110 // -XX--XX-
        0x00,      // 00000000 // --------
        0x00,      // 00000000 // --------
        0x00,      // 00000000 // --------
        0x00       // 00000000 // --------
      }
    },
```
Indeed it looks like hebrew letter gimel &#0x05D2 (U+05D2) https://unicodemap.org/details/0x05D2/index.html

There is no example of a wide symbol in the edk2 codebase, but it is very simple. `GlyphCol1` would encode left half of an image of the symbol, and `GlyphCol2` would encode the right half.

For example wide `A` can be something like this:
```
    {
      0x05d2,
      0x00,
      {
        0x00,      // 00000000 // --------
        0x00,      // 00000000 // --------
        0x00,      // 00000000 // --------
        0x01,      // 00000001 // -------X
        0x02,      // 00000010 // ------X-
        0x02,      // 00000010 // ------X-
        0x04,      // 00000100 // -----X--
        0x04,      // 00000100 // -----X--
        0x08,      // 00001000 // ----X---
        0x0F,      // 00001111 // ----XXXX
        0x10,      // 00010000 // ---X----
        0x10,      // 00010000 // ---X----
        0x20,      // 00100000 // --X-----
        0x20,      // 00100000 // --X-----
        0x70,      // 01110000 // -XXX----
        0x00,      // 00000000 // --------
        0x00,      // 00000000 // --------
        0x00,      // 00000000 // --------
        0x00       // 00000000 // --------
      },
      {
        0x00,      // 00000000 // --------
        0x00,      // 00000000 // --------
        0x00,      // 00000000 // --------
        0x00,      // 00000000 // --------
        0x80,      // 10000000 // X-------
        0x80,      // 10000000 // X-------
        0x40,      // 01000000 // -X------
        0x40,      // 01000000 // -X------
        0x20,      // 00100000 // --X-----
        0xE0,      // 11100000 // XXX-----
        0x10,      // 00010000 // ---X----
        0x10,      // 00010000 // ---X----
        0x08,      // 00001000 // ----X---
        0x08,      // 00001000 // ----X---
        0x1C,      // 00011100 // ---XXX--
        0x00,      // 00000000 // --------
        0x00,      // 00000000 // --------
        0x00,      // 00000000 // --------
        0x00       // 00000000 // --------
      },
      {
        0x00,
        0x00,
        0x00
      }
    },
```

# Get Glyph data for the Russian font

Honestly getting the Glyph data is a hardest part for adding a font.

First of all we need to find a pixel font with a size 8x19/16x19, which can already be a hard task.
I've found a `AST PremiumExec` font with cyrillic letters at the https://int10h.org/oldschool-pc-fonts/fontlist/font?ast_premiumexec
This font contains most of the symbols from the cyrillic unicode block (U+0400..U+04FF).
But this font is encoded in the *.woff format and we need somehow to transform it to the glyph data. I don't know any converters for such thing so I've used the idea of using the HTML canvas for this task from the https://github.com/zhenghuadai/uefi-programming/tree/master/book/GUIbasics/font. We print each symbol from the font on the HTML canvas one by one. For each symbol we grab an image bitmap and use its data to construct Glyph array that can be used in the UEFI environment. In the end we output Glyph array to the screen.

This is not a javascript lesson, but nevertheless I think the script deserves some explanation. First of all HTML part where we load font and declare canvas and script code:
```
<html>
  <head>
    <style>
      @font-face {
        font-family: "AST";
        src: url(web_ast_premiumexec.woff) format('woff');
      }
    </style>
  </head>
  <body>
    <canvas id="canvas" width="32" height="32"></canvas>
    <script type="text/javascript">
      ...
    </script>
  </body>
</html>
```

Our code is mainly this:
```
const unicode_start_code = 0x0400;
const unicode_end_code = 0x045F;
var f = new FontFace('AST', 'url(web_ast_premiumexec.woff)');
f.load().then(function() {
  document.write(UnicodeToGlyphs(unicode_start_code, unicode_end_code));
})
```

Once the font is loaded we execute our custom function `UnicodeToGlyphs` that would print unicode symbols from U+0400 to U+045F on a canvas, investigate the data and output final C array for the UEFI on the screen.

And here is the rest of the Javascript code. It is pretty simple, so I think it wouldn't be hard to understand, if it would be necessary:
```
function decimalToHex(d, padding) {
  var hex = Number(d).toString(16);
  padding = typeof (padding) === "undefined" || padding === null ? padding = 2 : padding;

  while (hex.length < padding) {
    hex = "0" + hex;
  }

  return hex;
}

function UnicodeToGlyphs(unicode_start_code, unicode_end_code) {
  const threshold = 100;                // `A` threshold to count data as a black pixel
  const FW = 16;
  const FH = 19;
  const left_glyph_start_column = 0;
  const left_glyph_end_column = FW/2 - 1;
  const right_glyph_start_column = FW/2;
  const right_glyph_end_column = FW - 1;

  const canvas = document.getElementById('canvas');
  canvas.width *= window.devicePixelRatio
  canvas.height *= window.devicePixelRatio
  canvas.style.width = 32
  canvas.style.height = 32
  const ctx = canvas.getContext('2d');
  ctx.strokeRect(0, 0, FW, FH);
  ctx.font = "19px AST"
  ctx.fillstyle='#00f';

  var wide_glyphs_str="EFI_WIDE_GLYPH  gSimpleFontWideGlyphData[] = {<BR>";
  var narrow_glyphs_str="EFI_NARROW_GLYPH  gSimpleFontNarrowGlyphData[] = {<BR>";
  for(i=unicode_start_code; i<=unicode_end_code; i++){
    wide_glyph = false;
    ctx.clearRect(0, 0, FW, FH);
    ctx.fillText(String.fromCharCode(i), 0, 0 + FW-1);
    var bitmapimg = ctx.getImageData(0, 0, FW, FH);
    var bitmap = bitmapimg.data;
    var left_glyph = " ";
    var right_glyph = " ";
    for (row=0; row<FH; row++){
      left_row = 0;
      for (col=left_glyph_start_column; col<=left_glyph_end_column; col++){
        left_row <<= 1
        if (bitmap[row*FW*4 + col*4 + 3] > threshold)
          left_row |= 1;
      }
      left_glyph += "0x" + decimalToHex(left_row);

      right_row = 0;
      for (col=right_glyph_start_column; col<=right_glyph_end_column; col++){
        right_row <<= 1
        if(bitmap[row*FW*4 + col*4 + 3] > threshold) {
          wide_glyph = true;
          right_row |= 1;
        }
      }
      right_glyph += "0x" + decimalToHex(right_row);

      if (row < (FH-1)) {
        left_glyph += ",";
        right_glyph += ",";
      }
    }
    if (wide_glyph)
      wide_glyphs_str += "{ 0x" + decimalToHex(i) + ", 0x00, " + "{" +  left_glyph + "}, {" + right_glyph + "}, {0x00,0x00,0x00}}," + "<BR>";
    else
      narrow_glyphs_str += "{ 0x" + decimalToHex(i) + ", 0x00, " + "{" +  left_glyph + "}},"+ "<BR>";
  }
  narrow_glyphs_str += "};<BR>"
  narrow_glyphs_str += "UINT32 gSimpleFontNarrowBytes = sizeof(gSimpleFontNarrowGlyphData);<BR>"
  wide_glyphs_str += "{ 0x00, 0x00, { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}" + "<BR>"
  wide_glyphs_str += "};<BR>"
  wide_glyphs_str += "UINT32 gSimpleFontWideBytes = sizeof(gSimpleFontWideGlyphData);<BR>"
  return wide_glyphs_str + "<BR>" + narrow_glyphs_str;
}
```

Put `create_font_data.html` and `web_ast_premiumexec.woff` next to each other and open `create_font_data.html`. I've used Chrome (just in case version is 95.0.4638.69) for this task.
Page should output something like this:
```
EFI_WIDE_GLYPH gSimpleFontWideGlyphData[] = {
{ 0x00, 0x00, { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}
};
UINT32 gSimpleFontWideBytes = sizeof(gSimpleFontWideGlyphData);

EFI_NARROW_GLYPH gSimpleFontNarrowGlyphData[] = {
{ 0x400, 0x00, { 0x60,0x30,0x00,0xfe,0x66,0x62,0x60,0x68,0x78,0x68,0x60,0x60,0x62,0x66,0xfe,0x00,0x00,0x00,0x00}},
{ 0x401, 0x00, { 0x66,0x66,0x00,0xfe,0x66,0x62,0x60,0x68,0x78,0x68,0x60,0x60,0x62,0x66,0xfe,0x00,0x00,0x00,0x00}},
{ 0x402, 0x00, { 0x00,0x00,0x00,0xfc,0x64,0x60,0x60,0x6c,0x76,0x66,0x66,0x66,0x66,0x66,0xe6,0x0c,0x00,0x00,0x00}},
{ 0x403, 0x00, { 0x0c,0x18,0x00,0xfe,0x66,0x62,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0xf0,0x00,0x00,0x00,0x00}},
 ...
{ 0x45c, 0x00, { 0x00,0x00,0x00,0x0c,0x18,0x30,0x00,0xe6,0x66,0x6c,0x78,0x78,0x6c,0x66,0xe6,0x00,0x00,0x00,0x00}},
{ 0x45d, 0x00, { 0x00,0x00,0x00,0x60,0x30,0x18,0x00,0xc6,0xc6,0xce,0xde,0xf6,0xe6,0xc6,0xc6,0x00,0x00,0x00,0x00}},
{ 0x45e, 0x00, { 0x00,0x00,0x00,0x00,0x6c,0x38,0x00,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0x7e,0x06,0x06,0x0c,0xf8,0x00}},
{ 0x45f, 0x00, { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xee,0x6c,0x6c,0x6c,0x6c,0x6c,0x6c,0xfe,0x10,0x10,0x00,0x00}},
};
UINT32 gSimpleFontNarrowBytes = sizeof(gSimpleFontNarrowGlyphData);
```
Our font is monospaced, so it only has 8x19 letters, therefore only `gSimpleFontNarrowGlyphData` array is filled with data. Everything looks ok, it is time to construct UEFI application and add font to the HII database.

