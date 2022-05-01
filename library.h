#include <iostream>
using namespace std;

/***    Cấu trúc file bmp gồm 4 phần chính:
 *      -   Header      : 14 bytes
 *                      + signature:     2 bytes
 *                      + filesize:      4 bytes
 *                      + reserved:      4 bytes
 *                      + DataOffset:    4 bytes
 *      -   DIB         : 40 bytes
 *                      + DIB size:      4 bytes
 *                      + width:         4 bytes
 *                      + height:        4 bytes
 *                      + planes:        2 bytes
 *                      + bpp:           2 bytes
 *                      + compression:   4 bytes
 *                      + ImageSize:     4 bytes
 *                      + XppM:          4 bytes
 *                      + YppM:          4 bytes
 *                      + Colors Used:   4 bytes
 *                      + ImportantColor:4 bytes
 *      -   Color Table : tùy biến
 *      -   Pixel Data  : tùy biến                  ***/

//  Khai báo cấu trúc Header:

typedef struct 
{
    short signature;    //  chữ ký file
    int fSize;          //  kích thước file
    int reserved;       //  phần dành riêng
    int DataOffset;     //  dịa chỉ bắt đầu lưu dữ liệu điểm ảnh
} HEADER;

//  Khai báo cấu trúc DIB:

typedef struct 
{
    int dibSize;        //  kích thước phần DIB
    int width;          //  số pixel theo chiều rộng
    int height;         //  số pixel theo chiều dài
    int planes;         //  số lớp màu (=1)
    short bpp;          //  số bit/pixel: 1,4,8,16,24,32
    int compression;    //  cách nén ảnh [0: không nén; 1: 8 bit RLE; 4 bit RLE]
    int ImageSize;      //  kích thước phần dữ liệu điểm ảnh
    int XppM;           //  độ phân giải theo phương ngang
    int YppM;           //  độ phân giải theo phương đứng
    int Colors;         //  số màu trong bảng màu
    int ImpColor;       //  số màu quan trọng
} DIB;

//  Khai báo cấu trúc ảnh BMP

typedef struct 
{
    HEADER *header;
    DIB *dib;
    char *color;
    char *PixelData;
} BITMAP;

