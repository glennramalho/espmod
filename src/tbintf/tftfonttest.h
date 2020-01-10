#ifndef _TFTFONTTEST
#define _TFTFONTTEST

#include <stdint.h>

uint32_t chartocrc(int font, unsigned char c);

#define CRC_F32_20 0x00000000
#define CRC_F32_21 0xf004488b
#define CRC_F32_22 0xcf004d91
#define CRC_F32_23 0xb849a419
#define CRC_F32_24 0x388e589a
#define CRC_F32_25 0x4e4aab37
#define CRC_F32_26 0xf4888f33
#define CRC_F32_27 0x78b6ff28
#define CRC_F32_28 0x67e0c4c0
#define CRC_F32_29 0xbbd3b31e
#define CRC_F32_2a 0xa1bc06f2
#define CRC_F32_2b 0x228a7689
#define CRC_F32_2c 0xe4cd1af8
#define CRC_F32_2d 0x8139af49
#define CRC_F32_2e 0x3fbec517
#define CRC_F32_2f 0xdb10b33d
#define CRC_F32_30 0x1dc7426b
#define CRC_F32_31 0x201b18fa
#define CRC_F32_32 0x18b11be5
#define CRC_F32_33 0xaec22211
#define CRC_F32_34 0x5773731c
#define CRC_F32_35 0x898d7651
#define CRC_F32_36 0x9162ecca
#define CRC_F32_37 0xe24ddd22
#define CRC_F32_38 0xc7d14a3d
#define CRC_F32_39 0x8003f493
#define CRC_F32_3a 0x3f8ae1b3
#define CRC_F32_3b 0xe4f93e5c
#define CRC_F32_3c 0xe56ac5fe
#define CRC_F32_3d 0x1ce8311a
#define CRC_F32_3e 0x3002ddc0
#define CRC_F32_3f 0xa3622585
#define CRC_F32_40 0x75daf6f1
#define CRC_F32_41 0x210471ae
#define CRC_F32_42 0xe6228e7b
#define CRC_F32_43 0x6e9192bb
#define CRC_F32_44 0xf91936f4
#define CRC_F32_45 0xc9396abd
#define CRC_F32_46 0x79570378
#define CRC_F32_47 0x3cd7669f
#define CRC_F32_48 0x4d8db41b
#define CRC_F32_49 0x452cf647
#define CRC_F32_4a 0x17de7e7d
#define CRC_F32_4b 0x9131ab9b
#define CRC_F32_4c 0x472486d7
#define CRC_F32_4d 0xd735919f
#define CRC_F32_4e 0xc29ceaa6
#define CRC_F32_4f 0xcdeae4bb
#define CRC_F32_50 0xeeffa394
#define CRC_F32_51 0x3a72a201
#define CRC_F32_52 0x593c4016
#define CRC_F32_53 0x8beaf8a6
#define CRC_F32_54 0x0eae5c4a
#define CRC_F32_55 0x1a90f9ef
#define CRC_F32_56 0x281ad2ef
#define CRC_F32_57 0x88a5aeeb
#define CRC_F32_58 0x1922a6c5
#define CRC_F32_59 0x2e6e516a
#define CRC_F32_5a 0x1c2633d9
#define CRC_F32_5b 0x463e79eb
#define CRC_F32_5c 0x678dea75
#define CRC_F32_5d 0x8ff68374
#define CRC_F32_5e 0x6b833ca0
#define CRC_F32_5f 0xe1f22467
#define CRC_F32_60 0x8b7cab90
#define CRC_F32_61 0x6e5613db
#define CRC_F32_62 0x36056f6c
#define CRC_F32_63 0x531e08e8
#define CRC_F32_64 0x5d540d24
#define CRC_F32_65 0x8bf681af
#define CRC_F32_66 0x758d0332
#define CRC_F32_67 0xf43841b0
#define CRC_F32_68 0xab268b15
#define CRC_F32_69 0xbe91bd93
#define CRC_F32_6a 0x9a56d38a
#define CRC_F32_6b 0x60d0f355
#define CRC_F32_6c 0x452cf647
#define CRC_F32_6d 0xc7e398f5
#define CRC_F32_6e 0xc1b3f043
#define CRC_F32_6f 0xa7246bbf
#define CRC_F32_70 0xf22ef669
#define CRC_F32_71 0x838a7265
#define CRC_F32_72 0x7007937c
#define CRC_F32_73 0x80e840f8
#define CRC_F32_74 0x15e0806f
#define CRC_F32_75 0xdc830a4a
#define CRC_F32_76 0x04943c5e
#define CRC_F32_77 0x33b434cb
#define CRC_F32_78 0xc25e1c20
#define CRC_F32_79 0xcb39f415
#define CRC_F32_7a 0x16062c26
#define CRC_F32_7b 0xbaac495d
#define CRC_F32_7c 0x04070c61
#define CRC_F32_7d 0x87268d90
#define CRC_F32_7e 0xc89dc183
#define CRC_F32_7f 0x00000000
#define CRC_F48_20 0x00000000
#define CRC_F48_21 0x00000000
#define CRC_F48_22 0x00000000
#define CRC_F48_23 0x00000000
#define CRC_F48_24 0x00000000
#define CRC_F48_25 0x00000000
#define CRC_F48_26 0x00000000
#define CRC_F48_27 0x00000000
#define CRC_F48_28 0x00000000
#define CRC_F48_29 0x00000000
#define CRC_F48_2a 0x00000000
#define CRC_F48_2b 0x00000000
#define CRC_F48_2c 0x00000000
#define CRC_F48_2d 0x2c560406
#define CRC_F48_2e 0x0b03adcc
#define CRC_F48_2f 0x00000000
#define CRC_F48_30 0x8a9e1501
#define CRC_F48_31 0x73b4c7c3
#define CRC_F48_32 0x66dadd39
#define CRC_F48_33 0x6ff09454
#define CRC_F48_34 0x054c3683
#define CRC_F48_35 0x5d0f4ab3
#define CRC_F48_36 0x4fc0efbc
#define CRC_F48_37 0xd675fc4c
#define CRC_F48_38 0xa6c81107
#define CRC_F48_39 0xb407b408
#define CRC_F48_3a 0x10cb81b9
#define CRC_F48_3b 0x00000000
#define CRC_F48_3c 0x00000000
#define CRC_F48_3d 0x00000000
#define CRC_F48_3e 0x00000000
#define CRC_F48_3f 0x00000000
#define CRC_F48_40 0x00000000
#define CRC_F48_41 0x00000000
#define CRC_F48_42 0x00000000
#define CRC_F48_43 0x00000000
#define CRC_F48_44 0x00000000
#define CRC_F48_45 0x00000000
#define CRC_F48_46 0x00000000
#define CRC_F48_47 0x00000000
#define CRC_F48_48 0x00000000
#define CRC_F48_49 0x00000000
#define CRC_F48_4a 0x00000000
#define CRC_F48_4b 0x00000000
#define CRC_F48_4c 0x00000000
#define CRC_F48_4d 0x00000000
#define CRC_F48_4e 0x00000000
#define CRC_F48_4f 0x00000000
#define CRC_F48_50 0x00000000
#define CRC_F48_51 0x00000000
#define CRC_F48_52 0x00000000
#define CRC_F48_53 0x00000000
#define CRC_F48_54 0x00000000
#define CRC_F48_55 0x00000000
#define CRC_F48_56 0x00000000
#define CRC_F48_57 0x00000000
#define CRC_F48_58 0x00000000
#define CRC_F48_59 0x00000000
#define CRC_F48_5a 0x00000000
#define CRC_F48_5b 0x00000000
#define CRC_F48_5c 0x00000000
#define CRC_F48_5d 0x00000000
#define CRC_F48_5e 0x00000000
#define CRC_F48_5f 0x00000000
#define CRC_F48_60 0x00000000
#define CRC_F48_61 0x00000000
#define CRC_F48_62 0x00000000
#define CRC_F48_63 0x00000000
#define CRC_F48_64 0x00000000
#define CRC_F48_65 0x00000000
#define CRC_F48_66 0x00000000
#define CRC_F48_67 0x00000000
#define CRC_F48_68 0x00000000
#define CRC_F48_69 0x00000000
#define CRC_F48_6a 0x00000000
#define CRC_F48_6b 0x00000000
#define CRC_F48_6c 0x00000000
#define CRC_F48_6d 0x00000000
#define CRC_F48_6e 0x00000000
#define CRC_F48_6f 0x00000000
#define CRC_F48_70 0x00000000
#define CRC_F48_71 0x00000000
#define CRC_F48_72 0x00000000
#define CRC_F48_73 0x00000000
#define CRC_F48_74 0x00000000
#define CRC_F48_75 0x00000000
#define CRC_F48_76 0x00000000
#define CRC_F48_77 0x00000000
#define CRC_F48_78 0x00000000
#define CRC_F48_79 0x00000000
#define CRC_F48_7a 0x00000000
#define CRC_F48_7b 0x00000000
#define CRC_F48_7c 0x00000000
#define CRC_F48_7d 0x00000000
#define CRC_F48_7e 0x00000000
#define CRC_F48_7f 0x00000000
#define CRC_F64_20 0x00000000
#define CRC_F64_21 0x00000000
#define CRC_F64_22 0x00000000
#define CRC_F64_23 0x00000000
#define CRC_F64_24 0x00000000
#define CRC_F64_25 0x00000000
#define CRC_F64_26 0x00000000
#define CRC_F64_27 0x00000000
#define CRC_F64_28 0x00000000
#define CRC_F64_29 0x00000000
#define CRC_F64_2a 0x00000000
#define CRC_F64_2b 0x00000000
#define CRC_F64_2c 0x00000000
#define CRC_F64_2d 0x0b8b5da2
#define CRC_F64_2e 0x8dac58b1
#define CRC_F64_2f 0x00000000
#define CRC_F64_30 0xf0c0d396
#define CRC_F64_31 0xc3b8d2d5
#define CRC_F64_32 0x315e5f49
#define CRC_F64_33 0x15585946
#define CRC_F64_34 0xc30e9f11
#define CRC_F64_35 0x0837a602
#define CRC_F64_36 0xccbb4dae
#define CRC_F64_37 0x1e95cd95
#define CRC_F64_38 0x685ca22e
#define CRC_F64_39 0xebed7635
#define CRC_F64_3a 0x6f5d2ff6
#define CRC_F64_3b 0x00000000
#define CRC_F64_3c 0x00000000
#define CRC_F64_3d 0x00000000
#define CRC_F64_3e 0x00000000
#define CRC_F64_3f 0x00000000
#define CRC_F64_40 0x00000000
#define CRC_F64_41 0x00000000
#define CRC_F64_42 0x00000000
#define CRC_F64_43 0x00000000
#define CRC_F64_44 0x00000000
#define CRC_F64_45 0x00000000
#define CRC_F64_46 0x00000000
#define CRC_F64_47 0x00000000
#define CRC_F64_48 0x00000000
#define CRC_F64_49 0x00000000
#define CRC_F64_4a 0x00000000
#define CRC_F64_4b 0x00000000
#define CRC_F64_4c 0x00000000
#define CRC_F64_4d 0x00000000
#define CRC_F64_4e 0x00000000
#define CRC_F64_4f 0x00000000
#define CRC_F64_50 0x00000000
#define CRC_F64_51 0x00000000
#define CRC_F64_52 0x00000000
#define CRC_F64_53 0x00000000
#define CRC_F64_54 0x00000000
#define CRC_F64_55 0x00000000
#define CRC_F64_56 0x00000000
#define CRC_F64_57 0x00000000
#define CRC_F64_58 0x00000000
#define CRC_F64_59 0x00000000
#define CRC_F64_5a 0x00000000
#define CRC_F64_5b 0x00000000
#define CRC_F64_5c 0x00000000
#define CRC_F64_5d 0x00000000
#define CRC_F64_5e 0x00000000
#define CRC_F64_5f 0x00000000
#define CRC_F64_60 0x00000000
#define CRC_F64_61 0x3641aacb
#define CRC_F64_62 0x00000000
#define CRC_F64_63 0x00000000
#define CRC_F64_64 0x00000000
#define CRC_F64_65 0x00000000
#define CRC_F64_66 0x00000000
#define CRC_F64_67 0x00000000
#define CRC_F64_68 0x00000000
#define CRC_F64_69 0x00000000
#define CRC_F64_6a 0x00000000
#define CRC_F64_6b 0x00000000
#define CRC_F64_6c 0x00000000
#define CRC_F64_6d 0x9a5bd9d3
#define CRC_F64_6e 0x00000000
#define CRC_F64_6f 0x00000000
#define CRC_F64_70 0xef72bc07
#define CRC_F64_71 0x00000000
#define CRC_F64_72 0x00000000
#define CRC_F64_73 0x00000000
#define CRC_F64_74 0x00000000
#define CRC_F64_75 0x00000000
#define CRC_F64_76 0x00000000
#define CRC_F64_77 0x00000000
#define CRC_F64_78 0x00000000
#define CRC_F64_79 0x00000000
#define CRC_F64_7a 0x00000000
#define CRC_F64_7b 0x00000000
#define CRC_F64_7c 0x00000000
#define CRC_F64_7d 0x00000000
#define CRC_F64_7e 0x00000000
#define CRC_F64_7f 0x00000000
#define CRC_F72_20 0x00000000
#define CRC_F72_21 0x00000000
#define CRC_F72_22 0x00000000
#define CRC_F72_23 0x00000000
#define CRC_F72_24 0x00000000
#define CRC_F72_25 0x00000000
#define CRC_F72_26 0x00000000
#define CRC_F72_27 0x00000000
#define CRC_F72_28 0x00000000
#define CRC_F72_29 0x00000000
#define CRC_F72_2a 0x00000000
#define CRC_F72_2b 0x00000000
#define CRC_F72_2c 0x00000000
#define CRC_F72_2d 0x814ef702
#define CRC_F72_2e 0x866e8dae
#define CRC_F72_2f 0x00000000
#define CRC_F72_30 0x8f56f9c5
#define CRC_F72_31 0x2d9d151b
#define CRC_F72_32 0xe2dfa078
#define CRC_F72_33 0xd1cef528
#define CRC_F72_34 0x7108ce79
#define CRC_F72_35 0x708a70f4
#define CRC_F72_36 0x5a3c9d9d
#define CRC_F72_37 0xac01c7ac
#define CRC_F72_38 0xe6b561ce
#define CRC_F72_39 0x9f7f2ffe
#define CRC_F72_3a 0xf8118001
#define CRC_F72_3b 0x00000000
#define CRC_F72_3c 0x00000000
#define CRC_F72_3d 0x00000000
#define CRC_F72_3e 0x00000000
#define CRC_F72_3f 0x00000000
#define CRC_F72_40 0x00000000
#define CRC_F72_41 0x00000000
#define CRC_F72_42 0x00000000
#define CRC_F72_43 0x00000000
#define CRC_F72_44 0x00000000
#define CRC_F72_45 0x00000000
#define CRC_F72_46 0x00000000
#define CRC_F72_47 0x00000000
#define CRC_F72_48 0x00000000
#define CRC_F72_49 0x00000000
#define CRC_F72_4a 0x00000000
#define CRC_F72_4b 0x00000000
#define CRC_F72_4c 0x00000000
#define CRC_F72_4d 0x00000000
#define CRC_F72_4e 0x00000000
#define CRC_F72_4f 0x00000000
#define CRC_F72_50 0x00000000
#define CRC_F72_51 0x00000000
#define CRC_F72_52 0x00000000
#define CRC_F72_53 0x00000000
#define CRC_F72_54 0x00000000
#define CRC_F72_55 0x00000000
#define CRC_F72_56 0x00000000
#define CRC_F72_57 0x00000000
#define CRC_F72_58 0x00000000
#define CRC_F72_59 0x00000000
#define CRC_F72_5a 0x00000000
#define CRC_F72_5b 0x00000000
#define CRC_F72_5c 0x00000000
#define CRC_F72_5d 0x00000000
#define CRC_F72_5e 0x00000000
#define CRC_F72_5f 0x00000000
#define CRC_F72_60 0x00000000
#define CRC_F72_61 0x00000000
#define CRC_F72_62 0x00000000
#define CRC_F72_63 0x00000000
#define CRC_F72_64 0x00000000
#define CRC_F72_65 0x00000000
#define CRC_F72_66 0x00000000
#define CRC_F72_67 0x00000000
#define CRC_F72_68 0x00000000
#define CRC_F72_69 0x00000000
#define CRC_F72_6a 0x00000000
#define CRC_F72_6b 0x00000000
#define CRC_F72_6c 0x00000000
#define CRC_F72_6d 0x00000000
#define CRC_F72_6e 0x00000000
#define CRC_F72_6f 0x00000000
#define CRC_F72_70 0x00000000
#define CRC_F72_71 0x00000000
#define CRC_F72_72 0x00000000
#define CRC_F72_73 0x00000000
#define CRC_F72_74 0x00000000
#define CRC_F72_75 0x00000000
#define CRC_F72_76 0x00000000
#define CRC_F72_77 0x00000000
#define CRC_F72_78 0x00000000
#define CRC_F72_79 0x00000000
#define CRC_F72_7a 0x00000000
#define CRC_F72_7b 0x00000000
#define CRC_F72_7c 0x00000000
#define CRC_F72_7d 0x00000000
#define CRC_F72_7e 0x00000000
#define CRC_F72_7f 0x00000000

#endif