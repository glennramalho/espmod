#include <stdio.h>
#include <Fonts/Font32rle.h>
#include <Fonts/Font64rle.h>
#include <Fonts/Font72rle.h>
#include <Fonts/Font7srle.h>
#include <crccalc.h>

void dotbl(FILE *cpp, FILE *h,
      const unsigned char* const chrtbl[], const unsigned char widtbl[],
      int nr_chrs, int chr_hgt, int pixsize, int firstchr) {
   int character;
   int num;
   int col;
   unsigned char value;
   int pos;
   uint32_t crc;

   /* The CPP has a bit of overhead. */
   fprintf(cpp, "\n   if (font == %d) switch(c) {\n", pixsize);

   /* And we generate the CRC codes for each char. */
   for(character = 0; character < nr_chrs; character = character + 1) {
      crc = 0;
      pos = 0;
      col = 0;
      do {
         value = chrtbl[character][col];
         num = (0x7fU & value)+1U;
         pos = pos + num;
         col = col + 1;
         while(num > 0) {
            crc = do1crc(crc, (value>=0x80)?0xff:0x00);
            crc = do1crc(crc, (value>=0x80)?0xff:0x00);
            num = num - 1;
         }
      } while (pos < chr_hgt * widtbl[character]);
      fprintf(h, "#define CRC_F%d_%02x 0x%08x\n", pixsize, character+firstchr,
         crc);
      if (crc != 0) fprintf(cpp, "      case 0x%02x: return CRC_F%d_%02x;\n",
         character+firstchr, pixsize, character+firstchr);
   }
   /* And for the CPP we close off the case. We set the default to return
    * 0 and that is because most chars are incomplete and therefore return
    * 0. */
   fprintf(cpp, "      default: return 0;\n");
   fprintf(cpp, "   }\n");
}

int main() {
   FILE *cpp, *h;

   /* We create the test function file and the header file. */
   cpp = fopen("tftfonttest.cpp", "w");
   if (cpp == NULL) {
      perror("tftfonttest.cpp");
      return 1;
   }
   /* And begin the file. */
   fprintf(cpp, "#include \"tftfonttest.h\"\n\n");
   fprintf(cpp, "uint32_t chartocrc(int font, unsigned char c) {\n");
   /* We do the same for the header. */
   h = fopen("tftfonttest.h", "w");
   if (h == NULL) {
      perror("tftfonttest.h");
      return 1;
   }
   fprintf(h, "#ifndef _TFTFONTTEST\n");
   fprintf(h, "#define _TFTFONTTEST\n\n");
   fprintf(h, "#include <stdint.h>\n\n");
   fprintf(h, "uint32_t chartocrc(int font, unsigned char c);\n\n");

   /* Now we generate the CRC Table. */
   crctable_init();

   /* And we calculate the codes and add them to the files. The header
    * will take the defines with all codes. The C file takes a case
    * with each non-zero codes.
    */
   dotbl(cpp,h,chrtbl_f32,widtbl_f32,nr_chrs_f32,chr_hgt_f32,32, firstchr_f32);
   dotbl(cpp,h,chrtbl_f7s,widtbl_f7s,nr_chrs_f7s,chr_hgt_f7s,48, firstchr_f7s);
   dotbl(cpp,h,chrtbl_f64,widtbl_f64,nr_chrs_f64,chr_hgt_f64,64, firstchr_f64);
   dotbl(cpp,h,chrtbl_f72,widtbl_f72,nr_chrs_f72,chr_hgt_f72,72, firstchr_f72);

   /* The C file we need to give something back if the customer asks for a
    * non-existant font. Then we close off the function and the file. */
   fprintf(cpp, "\n   return 0;\n");
   fprintf(cpp, "}\n");
   fclose(cpp);

   /* The header we only need to end the ifdef. */
   fprintf(h, "\n#endif\n");
   fclose(h);

   return 0;
}
