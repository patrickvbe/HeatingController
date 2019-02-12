// Created by http://oleddisplay.squix.ch/ Consider a donation
// In case of problems make sure that you are using the font file with the correct version!
const uint8_t Open_Sans_Condensed_Bold_40[] PROGMEM = {
	0x21, // Width: 33
	0x37, // Height: 55
	0x20, // First Char: 32
	0x1A, // Numbers of Chars: 26

	// Jump Table:
	0xFF, 0xFF, 0x00, 0x0A,  // 32:65535
	0x00, 0x00, 0x3E, 0x0B,  // 33:0
	0x00, 0x3E, 0x6B, 0x12,  // 34:62
	0x00, 0xA9, 0x90, 0x16,  // 35:169
	0x01, 0x39, 0x7C, 0x13,  // 36:313
	0x01, 0xB5, 0xC9, 0x1E,  // 37:437
	0x02, 0x7E, 0x99, 0x17,  // 38:638
	0x03, 0x17, 0x2D, 0x0A,  // 39:791
	0x03, 0x44, 0x4F, 0x0D,  // 40:836
	0x03, 0x93, 0x52, 0x0D,  // 41:915
	0x03, 0xE5, 0x7A, 0x13,  // 42:997
	0x04, 0x5F, 0x6D, 0x12,  // 43:1119
	0x04, 0xCC, 0x37, 0x0B,  // 44:1228
	0x05, 0x03, 0x52, 0x0D,  // 45:1283
	0x05, 0x55, 0x3E, 0x0B,  // 46:1365
	0x05, 0x93, 0x6B, 0x10,  // 47:1427
	0x05, 0xFE, 0x7B, 0x13,  // 48:1534
	0x06, 0x79, 0x61, 0x13,  // 49:1657
	0x06, 0xDA, 0x76, 0x13,  // 50:1754
	0x07, 0x50, 0x75, 0x13,  // 51:1872
	0x07, 0xC5, 0x7C, 0x13,  // 52:1989
	0x08, 0x41, 0x75, 0x13,  // 53:2113
	0x08, 0xB6, 0x7C, 0x13,  // 54:2230
	0x09, 0x32, 0x73, 0x13,  // 55:2354
	0x09, 0xA5, 0x7C, 0x13,  // 56:2469
	0x0A, 0x21, 0x75, 0x13,  // 57:2593

	// Font Data:
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x01,0x00,0xC0,0x03,0x00,0x00,0xC0,0xFF,0xFF,0xE1,0x07,0x00,0x00,0xC0,0xFF,0xFF,0xE1,0x07,0x00,0x00,0xC0,0xFF,0xFF,0xE1,0x07,0x00,0x00,0xC0,0xFF,0xFF,0xE1,0x07,0x00,0x00,0xC0,0xFF,0xFF,0xC1,0x03,0x00,0x00,0xC0,0x00,0x00,0x80,0x01,	// 33
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0xFF,0x00,0x00,0x00,0x00,0x00,0xC0,0xFF,0x00,0x00,0x00,0x00,0x00,0xC0,0xFF,0x00,0x00,0x00,0x00,0x00,0xC0,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0xFF,0x00,0x00,0x00,0x00,0x00,0xC0,0xFF,0x00,0x00,0x00,0x00,0x00,0xC0,0xFF,0x00,0x00,0x00,0x00,0x00,0xC0,0xFF,0x00,0x00,0x00,0x00,0x00,0x40,	// 34
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x07,0x00,0x00,0x00,0x00,0xC0,0x83,0x07,0x00,0x00,0x00,0x00,0xC0,0x83,0x07,0x04,0x00,0x00,0x00,0xC0,0x83,0xFF,0x07,0x00,0x00,0x00,0xC0,0xF3,0xFF,0x07,0x00,0x00,0x00,0xC0,0xFF,0xFF,0x07,0x00,0x00,0x80,0xFF,0xFF,0xFF,0x00,0x00,0x00,0xC0,0xFF,0xFF,0x07,0x00,0x00,0x00,0xC0,0xFF,0x83,0x07,0x00,0x00,0x00,0xC0,0xC7,0x83,0x07,0x00,0x00,0x00,0x00,0xC0,0x83,0xE7,0x07,0x00,0x00,0x00,0xC0,0xC3,0xFF,0x07,0x00,0x00,0x00,0xC0,0xFF,0xFF,0x07,0x00,0x00,0x00,0xFE,0xFF,0xFF,0x07,0x00,0x00,0xC0,0xFF,0xFF,0x0F,0x00,0x00,0x00,0xC0,0xFF,0x9F,0x07,0x00,0x00,0x00,0xC0,0xFF,0x83,0x07,0x00,0x00,0x00,0x40,0xC0,0x83,0x07,0x00,0x00,0x00,0x00,0xC0,0x83,0x07,0x00,0x00,0x00,0x00,0xC0,0x03,	// 35
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x78,0x00,0x00,0x00,0x00,0xF8,0x03,0xF8,0x00,0x00,0x00,0x00,0xFC,0x0F,0xF0,0x00,0x00,0x00,0x00,0xFE,0x0F,0xF0,0x01,0x00,0x00,0x00,0xFE,0x1F,0xF0,0x01,0x00,0x00,0x00,0x1F,0x3F,0xE0,0x01,0x00,0x00,0x00,0x0F,0x3E,0xE0,0x01,0x00,0x00,0xE0,0xFF,0xFF,0xFF,0x1F,0x00,0x00,0xE0,0xFF,0xFF,0xFF,0x1F,0x00,0x00,0xE0,0xFF,0xFF,0xFF,0x1F,0x00,0x00,0x00,0x0F,0xF8,0xE0,0x01,0x00,0x00,0x00,0x0F,0xF8,0xF1,0x01,0x00,0x00,0x00,0x1F,0xF0,0xFF,0x00,0x00,0x00,0x00,0x1E,0xF0,0xFF,0x00,0x00,0x00,0x00,0x1E,0xE0,0x7F,0x00,0x00,0x00,0x00,0x02,0xC0,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x06,	// 36
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF8,0x07,0x00,0x00,0x00,0x00,0x00,0xFF,0x3F,0x00,0x00,0x00,0x00,0x80,0xFF,0x7F,0x00,0x00,0x00,0x00,0x80,0xFF,0xFF,0x00,0x00,0x00,0x00,0xC0,0x0F,0xFC,0x00,0x00,0x00,0x00,0xC0,0x03,0xF0,0x00,0x00,0x00,0x00,0xC0,0x03,0xF0,0x00,0x04,0x00,0x00,0xC0,0xFF,0xFF,0x00,0x07,0x00,0x00,0x80,0xFF,0x7F,0xE0,0x07,0x00,0x00,0x00,0xFF,0x3F,0xF8,0x07,0x00,0x00,0x00,0xFE,0x1F,0xFF,0x03,0x00,0x00,0x00,0x00,0xC0,0xFF,0x00,0x00,0x00,0x00,0x00,0xF8,0x1F,0x00,0x00,0x00,0x00,0x00,0xFE,0x07,0x00,0x00,0x00,0x00,0xC0,0xFF,0x00,0x00,0x00,0x00,0x00,0xF8,0x3F,0x00,0x00,0x00,0x00,0x00,0xFE,0x07,0x06,0x00,0x00,0x00,0xC0,0xFF,0xF1,0xFF,0x00,0x00,0x00,0xC0,0x3F,0xFC,0xFF,0x03,0x00,0x00,0xC0,0x0F,0xFC,0xFF,0x03,0x00,0x00,0xC0,0x01,0xFE,0xFF,0x07,0x00,0x00,0x40,0x00,0x1E,0x80,0x07,0x00,0x00,0x00,0x00,0x1E,0x80,0x07,0x00,0x00,0x00,0x00,0x7E,0xF0,0x07,0x00,0x00,0x00,0x00,0xFC,0xFF,0x03,0x00,0x00,0x00,0x00,0xFC,0xFF,0x03,0x00,0x00,0x00,0x00,0xF0,0xFF,0x00,0x00,0x00,0x00,0x00,0x80,0x1F,	// 37
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3E,0x00,0x00,0x00,0x00,0x00,0x80,0xFF,0x00,0x00,0x00,0x00,0x38,0xE0,0xFF,0x01,0x00,0x00,0x00,0xFE,0xF1,0xFF,0x03,0x00,0x00,0x00,0xFF,0xF7,0xFF,0x03,0x00,0x00,0x80,0xFF,0xFF,0xF3,0x07,0x00,0x00,0xC0,0xFF,0x7F,0xC0,0x07,0x00,0x00,0xC0,0xC7,0x7F,0x80,0x07,0x00,0x00,0xC0,0x03,0xFE,0x80,0x07,0x00,0x00,0xC0,0x03,0xFF,0x83,0x07,0x00,0x00,0xC0,0x87,0xFF,0xC7,0x07,0x00,0x00,0xC0,0xFF,0xEF,0xDF,0x03,0x00,0x00,0x80,0xFF,0xC7,0xFF,0x03,0x00,0x00,0x80,0xFF,0x03,0xFF,0x01,0x00,0x00,0x00,0xFF,0x00,0xFE,0x01,0x00,0x00,0x00,0x38,0x00,0xFF,0x07,0x00,0x00,0x00,0x00,0xF0,0xFF,0x07,0x00,0x00,0x00,0x00,0xF0,0xFF,0x07,0x00,0x00,0x00,0x00,0xF0,0xCF,0x07,0x00,0x00,0x00,0x00,0xF0,0x03,0x07,0x00,0x00,0x00,0x00,0x70,0x00,0x06,	// 38
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0xFF,0x00,0x00,0x00,0x00,0x00,0xC0,0xFF,0x00,0x00,0x00,0x00,0x00,0xC0,0xFF,0x00,0x00,0x00,0x00,0x00,0xC0,0xFF,	// 39
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0x03,0x00,0x00,0x00,0x00,0x80,0xFF,0xFF,0x00,0x00,0x00,0x00,0xF0,0xFF,0xFF,0x07,0x00,0x00,0x00,0xFC,0xFF,0xFF,0x1F,0x00,0x00,0x00,0xFF,0xFF,0xFF,0x7F,0x00,0x00,0x80,0xFF,0x1F,0xF8,0xFF,0x00,0x00,0xC0,0x7F,0x00,0x00,0xFF,0x01,0x00,0xC0,0x0F,0x00,0x00,0xF8,0x01,0x00,0xC0,0x03,0x00,0x00,0xE0,0x01,0x00,0xC0,0x00,0x00,0x00,0x80,0x01,0x00,0x40,	// 40
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x01,0x00,0xC0,0x00,0x00,0x00,0x80,0x01,0x00,0xC0,0x03,0x00,0x00,0xE0,0x01,0x00,0xC0,0x0F,0x00,0x00,0xF8,0x01,0x00,0xC0,0x7F,0x00,0x00,0xFF,0x01,0x00,0x80,0xFF,0x1F,0xFC,0xFF,0x00,0x00,0x00,0xFE,0xFF,0xFF,0x7F,0x00,0x00,0x00,0xFC,0xFF,0xFF,0x1F,0x00,0x00,0x00,0xE0,0xFF,0xFF,0x07,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0xC0,0x03,	// 41
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x3E,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x04,0x00,0x00,0x00,0x00,0x00,0x3C,0x0E,0x00,0x00,0x00,0x00,0x00,0x3C,0x0F,0x00,0x00,0x00,0x00,0x00,0xF8,0x1F,0x00,0x00,0x00,0x00,0xE0,0xF8,0x0F,0x00,0x00,0x00,0x00,0xE0,0xFF,0x03,0x00,0x00,0x00,0x00,0xE0,0xFF,0x00,0x00,0x00,0x00,0x00,0xE0,0xFF,0x03,0x00,0x00,0x00,0x00,0xE0,0xF8,0x0F,0x00,0x00,0x00,0x00,0x00,0xF8,0x1F,0x00,0x00,0x00,0x00,0x00,0x3C,0x0F,0x00,0x00,0x00,0x00,0x00,0x3C,0x0E,0x00,0x00,0x00,0x00,0x00,0x3C,0x04,0x00,0x00,0x00,0x00,0x00,0x3E,0x00,0x00,0x00,0x00,0x00,0x00,0x30,	// 42
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x00,0xF0,0xFF,0x3F,0x00,0x00,0x00,0x00,0xF0,0xFF,0x3F,0x00,0x00,0x00,0x00,0xF0,0xFF,0x3F,0x00,0x00,0x00,0x00,0xF0,0xFF,0x3F,0x00,0x00,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x78,	// 43
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0x00,0x00,0xC0,0xFF,0x00,0x00,0x00,0x00,0x00,0xC0,0xFF,0x00,0x00,0x00,0x00,0x00,0xC0,0x7F,0x00,0x00,0x00,0x00,0x00,0xC0,0x0F,0x00,0x00,0x00,0x00,0x00,0xC0,0x03,	// 44
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x07,0x00,0x00,0x00,0x00,0x00,0xC0,0x07,0x00,0x00,0x00,0x00,0x00,0xC0,0x07,0x00,0x00,0x00,0x00,0x00,0xC0,0x07,0x00,0x00,0x00,0x00,0x00,0xC0,0x07,0x00,0x00,0x00,0x00,0x00,0xC0,0x07,0x00,0x00,0x00,0x00,0x00,0xC0,0x07,0x00,0x00,0x00,0x00,0x00,0xC0,0x07,0x00,0x00,0x00,0x00,0x00,0xC0,0x07,0x00,0x00,0x00,0x00,0x00,0xC0,0x07,0x00,0x00,0x00,0x00,0x00,0xC0,0x07,	// 45
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x03,0x00,0x00,0x00,0x00,0x00,0xE0,0x07,0x00,0x00,0x00,0x00,0x00,0xE0,0x07,0x00,0x00,0x00,0x00,0x00,0xE0,0x07,0x00,0x00,0x00,0x00,0x00,0xE0,0x07,0x00,0x00,0x00,0x00,0x00,0xC0,0x03,0x00,0x00,0x00,0x00,0x00,0x80,0x01,	// 46
	0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0xE0,0x07,0x00,0x00,0x00,0x00,0x00,0xFC,0x07,0x00,0x00,0x00,0x00,0x80,0xFF,0x07,0x00,0x00,0x00,0x00,0xE0,0xFF,0x07,0x00,0x00,0x00,0x00,0xFC,0xFF,0x00,0x00,0x00,0x00,0x80,0xFF,0x1F,0x00,0x00,0x00,0x00,0xF0,0xFF,0x03,0x00,0x00,0x00,0x00,0xFE,0x7F,0x00,0x00,0x00,0x00,0x80,0xFF,0x1F,0x00,0x00,0x00,0x00,0xC0,0xFF,0x03,0x00,0x00,0x00,0x00,0xC0,0x7F,0x00,0x00,0x00,0x00,0x00,0xC0,0x0F,0x00,0x00,0x00,0x00,0x00,0xC0,0x03,0x00,0x00,0x00,0x00,0x00,0x40,	// 47
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0x00,0x00,0x00,0x00,0x00,0xF0,0xFF,0x3F,0x00,0x00,0x00,0x00,0xFE,0xFF,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0x01,0x00,0x00,0x80,0xFF,0xFF,0xFF,0x03,0x00,0x00,0x80,0xFF,0xFF,0xFF,0x03,0x00,0x00,0xC0,0x0F,0x00,0xE0,0x07,0x00,0x00,0xC0,0x03,0x00,0x80,0x07,0x00,0x00,0xC0,0x03,0x00,0x80,0x07,0x00,0x00,0xC0,0x03,0x00,0x80,0x07,0x00,0x00,0xC0,0x0F,0x00,0xE0,0x07,0x00,0x00,0x80,0xFF,0xFF,0xFF,0x03,0x00,0x00,0x80,0xFF,0xFF,0xFF,0x03,0x00,0x00,0x00,0xFF,0xFF,0xFF,0x01,0x00,0x00,0x00,0xFE,0xFF,0xFF,0x00,0x00,0x00,0x00,0xF0,0xFF,0x1F,0x00,0x00,0x00,0x00,0x00,0x30,	// 48
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x70,0x00,0x00,0x00,0x00,0x00,0x00,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00,0x00,0x80,0x1F,0x00,0x00,0x00,0x00,0x00,0xC0,0xFF,0xFF,0xFF,0x07,0x00,0x00,0xC0,0xFF,0xFF,0xFF,0x07,0x00,0x00,0xC0,0xFF,0xFF,0xFF,0x07,0x00,0x00,0xC0,0xFF,0xFF,0xFF,0x07,0x00,0x00,0xC0,0xFF,0xFF,0xFF,0x07,	// 49
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x07,0x00,0x00,0x00,0x07,0x00,0xC0,0x07,0x00,0x00,0x00,0x0F,0x00,0xE0,0x07,0x00,0x00,0x80,0x0F,0x00,0xF8,0x07,0x00,0x00,0x80,0x07,0x00,0xFC,0x07,0x00,0x00,0xC0,0x07,0x00,0xFF,0x07,0x00,0x00,0xC0,0x03,0x80,0xFF,0x07,0x00,0x00,0xC0,0x03,0xE0,0x9F,0x07,0x00,0x00,0xC0,0x03,0xF0,0x8F,0x07,0x00,0x00,0xC0,0x07,0xFC,0x87,0x07,0x00,0x00,0xC0,0xFF,0xFF,0x81,0x07,0x00,0x00,0x80,0xFF,0xFF,0x80,0x07,0x00,0x00,0x80,0xFF,0x7F,0x80,0x07,0x00,0x00,0x00,0xFF,0x1F,0x80,0x07,0x00,0x00,0x00,0xFE,0x07,0x80,0x07,0x00,0x00,0x00,0x70,0x00,0x80,0x07,	// 50
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0xE0,0x03,0x00,0x00,0x00,0x03,0x00,0xC0,0x03,0x00,0x00,0x80,0x0F,0x00,0xC0,0x07,0x00,0x00,0x80,0x07,0x00,0x80,0x07,0x00,0x00,0xC0,0x07,0x3C,0x80,0x07,0x00,0x00,0xC0,0x03,0x3C,0x80,0x07,0x00,0x00,0xC0,0x03,0x3C,0x80,0x07,0x00,0x00,0xC0,0x03,0x3C,0x80,0x07,0x00,0x00,0xC0,0x03,0x7E,0x80,0x07,0x00,0x00,0xC0,0x07,0x7F,0xC0,0x07,0x00,0x00,0xC0,0xFF,0xFF,0xE0,0x03,0x00,0x00,0x80,0xFF,0xF7,0xFF,0x03,0x00,0x00,0x80,0xFF,0xF7,0xFF,0x03,0x00,0x00,0x00,0xFF,0xE3,0xFF,0x01,0x00,0x00,0x00,0xFE,0xC1,0xFF,0x00,0x00,0x00,0x00,0x38,0x80,0x3F,	// 51
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x0F,0x00,0x00,0x00,0x00,0x00,0xF0,0x0F,0x00,0x00,0x00,0x00,0x00,0xFC,0x0F,0x00,0x00,0x00,0x00,0x00,0xFF,0x0F,0x00,0x00,0x00,0x00,0xC0,0x7F,0x0F,0x00,0x00,0x00,0x00,0xF0,0x1F,0x0F,0x00,0x00,0x00,0x00,0xFC,0x07,0x0F,0x00,0x00,0x00,0x00,0xFF,0x01,0x0F,0x00,0x00,0x00,0xC0,0x7F,0x00,0x0F,0x00,0x00,0x00,0xC0,0x9F,0xFF,0xFF,0x07,0x00,0x00,0xC0,0xFF,0xFF,0xFF,0x07,0x00,0x00,0xC0,0xFF,0xFF,0xFF,0x07,0x00,0x00,0xC0,0xFF,0xFF,0xFF,0x07,0x00,0x00,0xC0,0xFF,0xFF,0xFF,0x07,0x00,0x00,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,	// 52
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xE0,0x03,0x00,0x00,0xC0,0xFF,0x1F,0xC0,0x03,0x00,0x00,0xC0,0xFF,0x3F,0xC0,0x07,0x00,0x00,0xC0,0xFF,0x1F,0x80,0x07,0x00,0x00,0xC0,0xFF,0x1F,0x80,0x07,0x00,0x00,0xC0,0x3F,0x1E,0x80,0x07,0x00,0x00,0xC0,0x03,0x1E,0x80,0x07,0x00,0x00,0xC0,0x03,0x1E,0x80,0x07,0x00,0x00,0xC0,0x03,0x3E,0xC0,0x07,0x00,0x00,0xC0,0x03,0x7E,0xF0,0x03,0x00,0x00,0xC0,0x03,0xFE,0xFF,0x03,0x00,0x00,0xC0,0x03,0xFC,0xFF,0x03,0x00,0x00,0xC0,0x03,0xF8,0xFF,0x01,0x00,0x00,0xC0,0x03,0xF0,0xFF,0x00,0x00,0x00,0x00,0x00,0xE0,0x3F,	// 53
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0x00,0x00,0x00,0x00,0x00,0xC0,0xFF,0x3F,0x00,0x00,0x00,0x00,0xF8,0xFF,0xFF,0x00,0x00,0x00,0x00,0xFC,0xFF,0xFF,0x01,0x00,0x00,0x00,0xFE,0xFF,0xFF,0x03,0x00,0x00,0x00,0xFF,0xF8,0xFF,0x03,0x00,0x00,0x80,0x1F,0x1C,0xE0,0x07,0x00,0x00,0x80,0x0F,0x0E,0x80,0x07,0x00,0x00,0xC0,0x07,0x0F,0x80,0x07,0x00,0x00,0xC0,0x03,0x0F,0x80,0x07,0x00,0x00,0xC0,0x03,0x1F,0xC0,0x07,0x00,0x00,0xC0,0x03,0xFF,0xFF,0x03,0x00,0x00,0xC0,0x03,0xFF,0xFF,0x03,0x00,0x00,0xC0,0x03,0xFE,0xFF,0x01,0x00,0x00,0xC0,0x03,0xFC,0xFF,0x00,0x00,0x00,0x00,0x00,0xF0,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x02,	// 54
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x03,0x00,0x00,0x00,0x00,0x00,0xC0,0x03,0x00,0x00,0x00,0x00,0x00,0xC0,0x03,0x00,0x00,0x07,0x00,0x00,0xC0,0x03,0x00,0xE0,0x07,0x00,0x00,0xC0,0x03,0x00,0xFC,0x07,0x00,0x00,0xC0,0x03,0x80,0xFF,0x07,0x00,0x00,0xC0,0x03,0xF0,0xFF,0x07,0x00,0x00,0xC0,0x03,0xFE,0xFF,0x03,0x00,0x00,0xC0,0xC3,0xFF,0x7F,0x00,0x00,0x00,0xC0,0xFB,0xFF,0x0F,0x00,0x00,0x00,0xC0,0xFF,0xFF,0x00,0x00,0x00,0x00,0xC0,0xFF,0x1F,0x00,0x00,0x00,0x00,0xC0,0xFF,0x03,0x00,0x00,0x00,0x00,0xC0,0x7F,0x00,0x00,0x00,0x00,0x00,0xC0,0x0F,	// 55
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x00,0x00,0x00,0x00,0x7C,0x80,0xFF,0x00,0x00,0x00,0x00,0xFF,0xE1,0xFF,0x01,0x00,0x00,0x80,0xFF,0xF3,0xFF,0x03,0x00,0x00,0x80,0xFF,0xFF,0xFF,0x03,0x00,0x00,0xC0,0xFF,0xFF,0xFB,0x07,0x00,0x00,0xC0,0x87,0x7F,0xC0,0x07,0x00,0x00,0xC0,0x03,0x3F,0x80,0x07,0x00,0x00,0xC0,0x03,0x1E,0x80,0x07,0x00,0x00,0xC0,0x03,0x3F,0x80,0x07,0x00,0x00,0xC0,0x87,0x7F,0xC0,0x07,0x00,0x00,0xC0,0xFF,0xFF,0xFB,0x07,0x00,0x00,0x80,0xFF,0xFB,0xFF,0x03,0x00,0x00,0x80,0xFF,0xF3,0xFF,0x03,0x00,0x00,0x00,0xFF,0xE1,0xFF,0x01,0x00,0x00,0x00,0x7C,0x80,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,	// 56
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x03,0x00,0x00,0x00,0x00,0x00,0xFC,0x3F,0x00,0x00,0x00,0x00,0x00,0xFE,0x7F,0x80,0x07,0x00,0x00,0x00,0xFF,0xFF,0x80,0x07,0x00,0x00,0x80,0xFF,0xFF,0x81,0x07,0x00,0x00,0xC0,0xFF,0xFF,0x81,0x07,0x00,0x00,0xC0,0x07,0xF0,0x81,0x07,0x00,0x00,0xC0,0x03,0xE0,0xC1,0x07,0x00,0x00,0xC0,0x03,0xE0,0xC1,0x07,0x00,0x00,0xC0,0x03,0xF0,0xE0,0x03,0x00,0x00,0xC0,0x0F,0x78,0xF8,0x03,0x00,0x00,0x80,0xFF,0xBF,0xFF,0x01,0x00,0x00,0x80,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,0x7F,0x00,0x00,0x00,0x00,0xFC,0xFF,0x1F,0x00,0x00,0x00,0x00,0xF0,0xFF,0x07,	// 57
};