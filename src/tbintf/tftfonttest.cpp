#include "tftfonttest.h"

uint32_t chartocrc(int font, unsigned char c) {

   if (font == 32) switch(c) {
      case 0x21: return CRC_F32_21;
      case 0x22: return CRC_F32_22;
      case 0x23: return CRC_F32_23;
      case 0x24: return CRC_F32_24;
      case 0x25: return CRC_F32_25;
      case 0x26: return CRC_F32_26;
      case 0x27: return CRC_F32_27;
      case 0x28: return CRC_F32_28;
      case 0x29: return CRC_F32_29;
      case 0x2a: return CRC_F32_2a;
      case 0x2b: return CRC_F32_2b;
      case 0x2c: return CRC_F32_2c;
      case 0x2d: return CRC_F32_2d;
      case 0x2e: return CRC_F32_2e;
      case 0x2f: return CRC_F32_2f;
      case 0x30: return CRC_F32_30;
      case 0x31: return CRC_F32_31;
      case 0x32: return CRC_F32_32;
      case 0x33: return CRC_F32_33;
      case 0x34: return CRC_F32_34;
      case 0x35: return CRC_F32_35;
      case 0x36: return CRC_F32_36;
      case 0x37: return CRC_F32_37;
      case 0x38: return CRC_F32_38;
      case 0x39: return CRC_F32_39;
      case 0x3a: return CRC_F32_3a;
      case 0x3b: return CRC_F32_3b;
      case 0x3c: return CRC_F32_3c;
      case 0x3d: return CRC_F32_3d;
      case 0x3e: return CRC_F32_3e;
      case 0x3f: return CRC_F32_3f;
      case 0x40: return CRC_F32_40;
      case 0x41: return CRC_F32_41;
      case 0x42: return CRC_F32_42;
      case 0x43: return CRC_F32_43;
      case 0x44: return CRC_F32_44;
      case 0x45: return CRC_F32_45;
      case 0x46: return CRC_F32_46;
      case 0x47: return CRC_F32_47;
      case 0x48: return CRC_F32_48;
      case 0x49: return CRC_F32_49;
      case 0x4a: return CRC_F32_4a;
      case 0x4b: return CRC_F32_4b;
      case 0x4c: return CRC_F32_4c;
      case 0x4d: return CRC_F32_4d;
      case 0x4e: return CRC_F32_4e;
      case 0x4f: return CRC_F32_4f;
      case 0x50: return CRC_F32_50;
      case 0x51: return CRC_F32_51;
      case 0x52: return CRC_F32_52;
      case 0x53: return CRC_F32_53;
      case 0x54: return CRC_F32_54;
      case 0x55: return CRC_F32_55;
      case 0x56: return CRC_F32_56;
      case 0x57: return CRC_F32_57;
      case 0x58: return CRC_F32_58;
      case 0x59: return CRC_F32_59;
      case 0x5a: return CRC_F32_5a;
      case 0x5b: return CRC_F32_5b;
      case 0x5c: return CRC_F32_5c;
      case 0x5d: return CRC_F32_5d;
      case 0x5e: return CRC_F32_5e;
      case 0x5f: return CRC_F32_5f;
      case 0x60: return CRC_F32_60;
      case 0x61: return CRC_F32_61;
      case 0x62: return CRC_F32_62;
      case 0x63: return CRC_F32_63;
      case 0x64: return CRC_F32_64;
      case 0x65: return CRC_F32_65;
      case 0x66: return CRC_F32_66;
      case 0x67: return CRC_F32_67;
      case 0x68: return CRC_F32_68;
      case 0x69: return CRC_F32_69;
      case 0x6a: return CRC_F32_6a;
      case 0x6b: return CRC_F32_6b;
      case 0x6c: return CRC_F32_6c;
      case 0x6d: return CRC_F32_6d;
      case 0x6e: return CRC_F32_6e;
      case 0x6f: return CRC_F32_6f;
      case 0x70: return CRC_F32_70;
      case 0x71: return CRC_F32_71;
      case 0x72: return CRC_F32_72;
      case 0x73: return CRC_F32_73;
      case 0x74: return CRC_F32_74;
      case 0x75: return CRC_F32_75;
      case 0x76: return CRC_F32_76;
      case 0x77: return CRC_F32_77;
      case 0x78: return CRC_F32_78;
      case 0x79: return CRC_F32_79;
      case 0x7a: return CRC_F32_7a;
      case 0x7b: return CRC_F32_7b;
      case 0x7c: return CRC_F32_7c;
      case 0x7d: return CRC_F32_7d;
      case 0x7e: return CRC_F32_7e;
      default: return 0;
   }

   if (font == 48) switch(c) {
      case 0x2d: return CRC_F48_2d;
      case 0x2e: return CRC_F48_2e;
      case 0x30: return CRC_F48_30;
      case 0x31: return CRC_F48_31;
      case 0x32: return CRC_F48_32;
      case 0x33: return CRC_F48_33;
      case 0x34: return CRC_F48_34;
      case 0x35: return CRC_F48_35;
      case 0x36: return CRC_F48_36;
      case 0x37: return CRC_F48_37;
      case 0x38: return CRC_F48_38;
      case 0x39: return CRC_F48_39;
      case 0x3a: return CRC_F48_3a;
      default: return 0;
   }

   if (font == 64) switch(c) {
      case 0x2d: return CRC_F64_2d;
      case 0x2e: return CRC_F64_2e;
      case 0x30: return CRC_F64_30;
      case 0x31: return CRC_F64_31;
      case 0x32: return CRC_F64_32;
      case 0x33: return CRC_F64_33;
      case 0x34: return CRC_F64_34;
      case 0x35: return CRC_F64_35;
      case 0x36: return CRC_F64_36;
      case 0x37: return CRC_F64_37;
      case 0x38: return CRC_F64_38;
      case 0x39: return CRC_F64_39;
      case 0x3a: return CRC_F64_3a;
      case 0x61: return CRC_F64_61;
      case 0x6d: return CRC_F64_6d;
      case 0x70: return CRC_F64_70;
      default: return 0;
   }

   if (font == 72) switch(c) {
      case 0x2d: return CRC_F72_2d;
      case 0x2e: return CRC_F72_2e;
      case 0x30: return CRC_F72_30;
      case 0x31: return CRC_F72_31;
      case 0x32: return CRC_F72_32;
      case 0x33: return CRC_F72_33;
      case 0x34: return CRC_F72_34;
      case 0x35: return CRC_F72_35;
      case 0x36: return CRC_F72_36;
      case 0x37: return CRC_F72_37;
      case 0x38: return CRC_F72_38;
      case 0x39: return CRC_F72_39;
      case 0x3a: return CRC_F72_3a;
      default: return 0;
   }

   return 0;
}
