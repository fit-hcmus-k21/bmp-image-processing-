#include <iostream>
#include <math.h>
#include <string.h>
#include <fstream>
#pragma pack(2)
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
    char sign1;         //  chữ ký file
    char sign2;
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
    short planes;       //  số lớp màu (=1)
    short bpp;          //  số bit/pixel: 1,4,8,16,24,32
    int compression;    //  cách nén ảnh [0: không nén; 1: 8 bit RLE; 4 bit RLE]
    int ImageSize;      //  kích thước phần dữ liệu điểm ảnh
    int XppM;           //  độ phân giải theo phương ngang
    int YppM;           //  độ phân giải theo phương đứng
    int Colors;         //  số màu trong bảng màu
    int ImpColor;       //  số màu quan trọng
} DIB;

//  Khai báo cấu trúc pixel

typedef struct 
{
    char A;
    char B;             //  Blue
    char G;             //  Green
    char R;             //  Red
}  PIXEL_32;            //  Kiểu pixel ảnh 32bpp

typedef struct 
{
    char B;             //  Blue
    char G;             //  Green
    char R;             //  Red
}  PIXEL_24;            //  Kiểu pixel ảnh 24bpp

typedef struct 
{
    char C;                     
}  PIXEL_8;             //  Kiểu pixel ảnh 8bpp


//  Khai báo cấu trúc ảnh BMP

typedef struct 
{
    HEADER header;
    DIB dib;
    unsigned char *ColorsTable = NULL;  //  con trỏ tới vùng nhớ lưu thông tin bảng màu (nếu có)
    unsigned char *overage = NULL;      //  con trỏ tới vùng nhớ lưu phần dư của DIB (nếu có)
    unsigned char *PixelData =   NULL;    //  con trỏ tới vùng nhớ lưu dữ liệu điểm ảnh
} BITMAP;

//  Hàm đọc ảnh bmp từ đường dẫn file
void ReadBmp(char *FileInput, BITMAP &image)
{
    ifstream fin(FileInput,ios:: in | ios:: binary);
    
    //  Kiểm tra mở file thành công hay không
    if (!fin.is_open())
    {
        cout<<"-----Loi mo file-----\n";
        exit(225);
    }

    //  Đọc header ảnh
    fin.read((char *) &image.header,14);
    
    //  Đọc phần DIB
    fin.read((char*) &image.dib,sizeof(DIB));

    //  Kiểm tra xem kích thước DIB có bằng 40 hay không, nếu lớn hơn 40 thì đọc phần thừa vào overage
    if (image.dib.dibSize>40)
    {
        image.overage = new unsigned char[image.dib.dibSize-40];             //  1. Nhớ giải phóng bộ nhớ
        fin.read((char *) image.overage,image.dib.dibSize-40);
    }

    //  Cấp phát vùng nhớ cho con trỏ đến dữ liệu điểm ảnh
    image.PixelData = new unsigned char[image.dib.ImageSize];                //  2. Nhớ giải phóng bộ nhớ     

    //  Đưa con trỏ đến phần bắt đầu dữ liệu điểm ảnh
    fin.seekg(image.header.DataOffset); 

    //  Đọc phần dữ liệu điểm ảnh
    fin.read((char *) image.PixelData, image.dib.ImageSize);

    /***          Đọc thông tin bảng màu (nếu có)           ***/
    //  Tính kích thước bảng màu
    int size = image.header.DataOffset-image.dib.dibSize-sizeof(HEADER);
    if (size>0)
    {
        //  Cấp phát vùng nhớ để lưu thông tin bảng màu
        image.ColorsTable = new unsigned char[size];                            //  3. Nhớ giải phóng bộ nhớ
        fin.seekg(image.header.DataOffset-size);
        fin.read((char*) image.ColorsTable,size);
    }

    //  Đóng file
    fin.close();

}

//  Hàm lưu ảnh bmp xuống file theo đường dẫn
void SaveBmp(char *FileOutput, BITMAP &image)
{
    ofstream fout(FileOutput, ios:: out | ios::binary);

    //  Kiểm tra mở file
    if (!fout.is_open())
    {
        cout<<"\nLoi mo file...\n";
        exit(225);
    }

    //  Ghi header file
    fout.write((char *) &image.header, 14);

    //  Ghi phần DIB
    fout.write((char *) &image.dib,40);
    
    //  Ghi phần thừa của DIB (nếu có)
    if (image.overage != NULL)
    {
        fout.write((char *)image.overage,image.dib.dibSize-40);
    }

    //  Ghi thông tin bảng màu (nếu có)
    if (image.ColorsTable != NULL)
    {
        int R = image.header.DataOffset-image.dib.dibSize-sizeof(HEADER);
        fout.write((char*) image.ColorsTable,R);
    }
    
    //  Ghi dữ  liệu điểm ảnh
    fout.write((char *) image.PixelData, image.dib.ImageSize);
    
    //  Đóng file
    fout.close();
}

//   Hàm chuyển đổi ảnh 24bpp hay 32bpp sang ảnh 8bpp
void ConvertBmp(BITMAP img1, BITMAP &img2)
{
    //  Chuyển các thông tin ảnh nguồn cho ảnh đích
    img2.header = img1.header;
    img2.dib = img1.dib;

    //  Cài đặt một số thông tin cho ảnh 8bpp
    img2.dib.bpp=8;                                                 
    img2.dib.XppM=0;
    img2.dib.YppM=0;
    img2.overage=NULL;
    img2.dib.dibSize=40;
    img2.dib.compression=0;

    /***        Thông tin về bảng màu của ảnh 8bpp
     *  Bảng màu là tập hợp các màu được sử dụng trong ảnh, mỗi một màu là một entry
     *  được lưu trữ bằng 4bytes, mỗi một màu được lưu trữ 1byte còn 1byte dự trữ
     *  thứ tự là Blue, Green, Red, Reserved
     *  => kích thước bảng màu là : size = (số lượng màu)*4;
     *  [Trong đó, số lượng màu là 2^n (với n là số bpp)]
     *                                                                              ***/

    //  Tính số lượng màu của ảnh 8bpp
    img2.dib.Colors=pow(2,img2.dib.bpp);

    //  Tính kích thước của bảng màu
    int size = img2.dib.Colors*4;
    
    //  Cấp phát vùng nhớ để lưu thông tin bảng màu cho ảnh 8bpp
    img2.ColorsTable=new unsigned char[size];                                //  4.  Nhớ giải phóng bộ nhớ
    
    //  Tạo thông tin bảng màu cho ảnh 8bpp
    int d=0;
    for (int i=0;i<256;i++)
    {
        img2.ColorsTable[d++] = (unsigned char) i;
        img2.ColorsTable[d++] = (unsigned char) i;
        img2.ColorsTable[d++] = (unsigned char) i;
        img2.ColorsTable[d++] = 0;
    }

    //  Cập nhật lại vị trí bắt đầu dữ liệu điểm ảnh trên ảnh 8bpp
    img2.header.DataOffset = sizeof(HEADER) + img2.dib.dibSize + size;

    /***       Cách tính kích thước ảnh
     *  Padding bytes:
     *  Khi một mảng các pixel được nạp vào bộ nhớ, mỗi hàng phải bắt đầu tại một địa chỉ bộ nhớ mà địa chỉ
     *  đó là bội số của 4, do ta sử dụng pixel có thể kết thúc với địa chỉ không chia hết cho 4, vì vậy 
     *  sẽ có những padding bytes để bù đắp số byte thiếu và đảm bảo rằng kết thúc mỗi dòng địa chỉ bộ nhớ 
     *  luôn là bội của 4.
     *  => padding = [ 4-(width*bpp/8)%4 ]%4
     *     [Trong đó: bpp là số bit/pixel]
     *  => ImageSize = width*height*bpp/8+padding*height                     ***/

    //  Tính số padding bytes
    int padding1=(4-(img1.dib.width*img1.dib.bpp/8)%4)%4;       //  padding bytes ảnh nguồn
    int padding2=(4-(img2.dib.width*img2.dib.bpp/8)%4)%4;       //  padding bytes ảnh đích

    //  Tính lại kích thước ảnh
    img2.dib.ImageSize=(img2.dib.width*img2.dib.height*img2.dib.bpp/8) + padding2*img2.dib.height;

    //  Tính lại kích thước file
    img2.header.fSize = img2.header.DataOffset + img2.dib.ImageSize;

    //  Cấp phát vùng nhớ cho dữ liệu điểm ảnh của ảnh đích
    img2.PixelData=new unsigned char[img2.dib.ImageSize];                        //  5. Nhớ giải phóng bộ nhớ

    /****  Chuyển dữ liệu điểm ảnh                  ****/

    //  Khai báo con trỏ PixPtr đi qua từng pixel của ảnh nguồn
    unsigned char *PixPtr=img1.PixelData;

    //  Chuyển dữ liệu điểm ảnh từ ảnh nguồn (24bpp/32bpp) sang ảnh đích 8bpp
    unsigned char B,G,R;
    int index=0;

    
    if (img1.dib.bpp==24)       //  Đối với ảnh 24bit
    {   
        //  Đi qua từng dòng pixel (height)
        for (int i=0;i<img2.dib.height;i++)
        {
            //  Đi qua từng cột pixel (width)
            for (int j=0;j<img2.dib.width;j++)
            {
                //  Đi qua các bytes màu Blue, Green, Red
                B = *PixPtr++;
                G = *PixPtr++;
                R = *PixPtr++;
                //  Dữ liệu điểm ảnh của ảnh 8bpp bằng trung bình cộng 3 màu B,G,R của ảnh 24bpp hay 32bpp
                img2.PixelData[index++] = (B+G+R)/3;
            }
            //  Bỏ qua padding (bytes)
            PixPtr = PixPtr + padding1;
            index+=padding2;
        }
    }
    else

    if (img1.dib.bpp==32)       //  Đối với ảnh 32bit
    {
        //  Đi qua từng dòng pixel (height)
        for (int i=0;i<img2.dib.height;i++)
        {
            //  Đi qua từng byte trên cột pixel (width)
            for (int j=0;j<img2.dib.width;j++)
            {
                PixPtr++;                               //      Bỏ qua byte màu A rồi đi qua các bytes màu Blue, Green, Red
                B = *PixPtr++;
                G = *PixPtr++;
                R = *PixPtr++;
                //  Dữ liệu điểm ảnh của ảnh 8bpp bằng trung bình cộng 3 màu B,G,R của ảnh 24bpp hay 32bpp
                img2.PixelData[index++] = (B+G+R)/3;
                
            }
            //  Bỏ qua padding (bytes)
            PixPtr = PixPtr + padding1;
            index+=padding2;
        }
    }

}

//  Hàm thu nhỏ ảnh màu 32bpp, 24bpp, 8bpp theo tỉ lệ S cho trước
void ZoomOutBmp(BITMAP &img1, BITMAP &img2, int s)
{
    //  Chuyển thông tin ảnh cũ sang ảnh mới
    img2.header = img1.header;
    img2.dib = img1.dib;
    //  Cấp phát vùng nhớ
    if (img1.overage!=NULL)
    {
        img2.overage = new unsigned char[img2.dib.dibSize-sizeof(HEADER)-40];
        img2.overage = img1.overage;
    }
    if (img1.ColorsTable!=NULL)
    {
        img2.ColorsTable = new unsigned char[img2.header.DataOffset-sizeof(HEADER)-img2.dib.dibSize];
        img2.ColorsTable = img1.ColorsTable;
    }

    //  Tính lại width, height
    if (img1.dib.width % s == 0)
    {
        img2.dib.width = img1.dib.width/s;
    }
    else
    {
        img2.dib.width = img1.dib.width/s + 1;
    }
    if (img1.dib.height % s ==0)
    {
        img2.dib.height = img1.dib.height/s;
    }
    else
    {
        img2.dib.height = img1.dib.height/s +1;
    }

    //  Tính padding byte(s)
    int padding1 = (4-(img1.dib.width*img1.dib.bpp/8)%4)%4;             //  padding của ảnh cũ
    int padding2 = (4-(img2.dib.width*img2.dib.bpp/8)%4)%4;             //  padding của ảnh mới

    //  Tính lại image size và file size
    img2.dib.ImageSize = img2.dib.width*img2.dib.height*img2.dib.bpp/8 + padding2*img2.dib.height;
    img2.header.fSize = img2.header.DataOffset + img2.dib.ImageSize;

    //  Cấp phát vùng nhớ để lưu dữ liệu điểm ảnh trong ảnh mới
    img2.PixelData = new unsigned char[img2.dib.ImageSize];                      //  6. Nhớ giải phóng bộ nhớ

    //  Xử lí
    int wSrc=img1.dib.width;
    int hSrc=img1.dib.height;

    if (img1.dib.bpp==32)       //  Trường hợp 32 bit
    {
        //  Chuyển dữ liệu điểm ảnh qua mảng hai chiều các pixel
        PIXEL_32 **pData=new PIXEL_32*[img1.dib.height];
        for (int i=0;i<img1.dib.height;i++)
        {
            pData[i]=new PIXEL_32[img1.dib.width];
        }

        //  Khai báo con trỏ trỏ vào vùng nhớ lưu dữ liệu điểm ảnh của ảnh nguồn
        unsigned char *ptr=img1.PixelData;
        for (int i=0;i<img1.dib.height;i++)
        {
            for (int j=0;j<img1.dib.width;j++)
            {
                pData[i][j].A=*ptr++;
                pData[i][j].B=*ptr++;
                pData[i][j].G=*ptr++;
                pData[i][j].R=*ptr++;
            }
            ptr+=padding1;      // bỏ qua phần padding
        }   

        //  Câp phát vùng nhớ cho mảng để lưu dữ liệu điểm ảnh sau khi xử lí trên mảng 2 chiều của ảnh nguồn
        PIXEL_32 **pDataDst=new PIXEL_32*[img2.dib.height];
        for (int i=0;i<img2.dib.height;i++)
        {
            pDataDst[i]=new PIXEL_32[img2.dib.width];
        }

        //  Xử lí và chuyển dữ liệu điểm ảnh sang dạng mảng 2 chiều cái pixel với cách thức: điểm (x;y) trong ảnh đích sẽ là điểm (s*x;s*y) trong ảnh nguồn
        for (int i=0;i<img2.dib.height;i++)
        {
            for (int j=0;j<img2.dib.width;j++)
            {
                if (i*s<img1.dib.height && j*s<img1.dib.width)  //  Kiểm tra liệu điểm (s*x;s*y) có thuộc ma trận điểm ảnh nguồn không, nếu không thì lùi tọa độ lại
                {
                    pDataDst[i][j].A=pData[i*s][j*s].A;
                    pDataDst[i][j].B=pData[i*s][j*s].B;
                    pDataDst[i][j].G=pData[i*s][j*s].G;
                    pDataDst[i][j].R=pData[i*s][j*s].R;
                }
                else
                {
                    if (i*s>img1.dib.height && j*s>img1.dib.width)
                    {
                        pDataDst[i][j].A=pData[hSrc-1][wSrc-1].A;
                        pDataDst[i][j].B=pData[hSrc-1][wSrc-1].B;
                        pDataDst[i][j].G=pData[hSrc-1][wSrc-1].G;
                        pDataDst[i][j].R=pData[hSrc-1][wSrc-1].R;
                    }
                    else
                    {
                        if (i*s>img1.dib.height)
                        {
                            pDataDst[i][j].A=pData[hSrc-1][j*s].A;
                            pDataDst[i][j].B=pData[hSrc-1][j*s].B;
                            pDataDst[i][j].G=pData[hSrc-1][j*s].G;
                            pDataDst[i][j].R=pData[hSrc-1][j*s].R;
                        }
                        else
                        {       
                            pDataDst[i][j].A=pData[i*s][wSrc-1].A;
                            pDataDst[i][j].B=pData[i*s][wSrc-1].B;
                            pDataDst[i][j].G=pData[i*s][wSrc-1].G;
                            pDataDst[i][j].R=pData[i*s][wSrc-1].R;
                        }
                    }
                }
            
            }

    }

        //  Chuyển lại dữ liệu điểm ảnh dưới dạng mảng 1 chiều bao gồm padding bytes
        int index=0;
        for (int i=0;i<img2.dib.height;i++)
        {
            for (int j=0;j<img2.dib.width;j++)
            {
                img2.PixelData[index++]=pDataDst[i][j].A;
                img2.PixelData[index++]=pDataDst[i][j].B;
                img2.PixelData[index++]=pDataDst[i][j].G;
                img2.PixelData[index++]=pDataDst[i][j].R;
            }
            index+=padding2;
        }

        //  Giải phóng bộ nhớ đã cấp phát
        for (int i=0;i<img1.dib.height;i++)
        {
            delete[] pData[i];
        }
        delete[] pData;

        for (int i=0;i<img2.dib.height;i++)
        {
            delete[] pDataDst[i];
        }
        delete[] pDataDst;
    }
    else
    {
        //  Xử lí tương tự như ảnh 32bpp
        if (img1.dib.bpp==24)       //  Trường hợp 24 bit
        {
            PIXEL_24 **pData=new PIXEL_24*[img1.dib.height];
            for (int i=0;i<img1.dib.height;i++)
            {
                pData[i]=new PIXEL_24[img1.dib.width];
            }

            unsigned char *ptr=img1.PixelData;
            for (int i=0;i<img1.dib.height;i++)
            {
                for (int j=0;j<img1.dib.width;j++)
                {
                    pData[i][j].B=*ptr++;
                    pData[i][j].G=*ptr++;
                    pData[i][j].R=*ptr++;
                }
                ptr+=padding1;
            }

            PIXEL_24 **pDataDst=new PIXEL_24*[img2.dib.height];
            for (int i=0;i<img2.dib.height;i++)
            {
                pDataDst[i]=new PIXEL_24[img2.dib.width];
            }

            for (int i=0;i<img2.dib.height;i++)
            {
                for (int j=0;j<img2.dib.width;j++)
                {
                    if (i*s<img1.dib.height && j*s<img1.dib.width)
                    {
                        pDataDst[i][j].B=pData[i*s][j*s].B;
                        pDataDst[i][j].G=pData[i*s][j*s].G;
                        pDataDst[i][j].R=pData[i*s][j*s].R;
                    }
                    else
                    {
                        if (i*s>img1.dib.height && j*s>img1.dib.width)
                        {
                            pDataDst[i][j].B=pData[hSrc-1][wSrc-1].B;
                            pDataDst[i][j].G=pData[hSrc-1][wSrc-1].G;
                            pDataDst[i][j].R=pData[hSrc-1][wSrc-1].R;
                        }
                        else
                        {
                            if (i*s>img1.dib.height)
                            {
                                pDataDst[i][j].B=pData[hSrc-1][j*s].B;
                                pDataDst[i][j].G=pData[hSrc-1][j*s].G;
                                pDataDst[i][j].R=pData[hSrc-1][j*s].R;
                            }
                            else
                            {       
                                pDataDst[i][j].B=pData[i*s][wSrc-1].B;
                                pDataDst[i][j].G=pData[i*s][wSrc-1].G;
                                pDataDst[i][j].R=pData[i*s][wSrc-1].R;
                            }
                        }
                    }
            
                }

            }

            int index=0;
            for (int i=0;i<img2.dib.height;i++)
            {
                for (int j=0;j<img2.dib.width;j++)
                {  
                    img2.PixelData[index++]=pDataDst[i][j].B;
                    img2.PixelData[index++]=pDataDst[i][j].G;
                    img2.PixelData[index++]=pDataDst[i][j].R;
                }
                index+=padding2;
            }

            for (int i=0;i<img1.dib.height;i++)
            {
                delete[] pData[i];
            }
            delete[] pData;

            for (int i=0;i<img2.dib.height;i++)
            {
                delete[] pDataDst[i];
            }
            delete[] pDataDst;
            

        }
        else
        {
            //  Trường hợp 8 bit
            PIXEL_8 **pData=new PIXEL_8*[img1.dib.height];
            for (int i=0;i<img1.dib.height;i++)
            {
                pData[i]=new PIXEL_8[img1.dib.width];
            }

            unsigned char *ptr=img1.PixelData;
            for (int i=0;i<img1.dib.height;i++)
            {
                for (int j=0;j<img1.dib.width;j++)
                {
                
                    pData[i][j].C=*ptr++;
                }
                ptr+=padding1;
            }

            PIXEL_8 **pDataDst=new PIXEL_8*[img2.dib.height];
            for (int i=0;i<img2.dib.height;i++)
            {
                pDataDst[i]=new PIXEL_8[img2.dib.width];
            }

            for (int i=0;i<img2.dib.height;i++)
            {        
                for (int j=0;j<img2.dib.width;j++)
                {
                    if (i*s<img1.dib.height && j*s<img1.dib.width)
                    {
                        pDataDst[i][j].C=pData[i*s][j*s].C;
                    }
                    else
                    {
                        if (i*s>img1.dib.height && j*s>img1.dib.width)
                        {
                            pDataDst[i][j].C=pData[hSrc-1][wSrc-1].C;
                        }
                        else
                        {
                            if (i*s>img1.dib.height)
                            {
                                pDataDst[i][j].C=pData[hSrc-1][j*s].C;
                            }
                            else
                            {       
                                pDataDst[i][j].C=pData[i*s][wSrc-1].C;
                            }
                        }
                    }
            
                }

            }

            int index=0;
            for (int i=0;i<img2.dib.height;i++)
            {
                for (int j=0;j<img2.dib.width;j++)
                {
                    img2.PixelData[index++]=pDataDst[i][j].C;
                }
                index+=padding2;
            }

            for (int i=0;i<img1.dib.height;i++)
            {
                delete[] pData[i];
            }
            delete[] pData;

            for (int i=0;i<img2.dib.height;i++)
            {
                delete[] pDataDst[i];
            }
            delete[] pDataDst;
        }
    }
}

//  Hàm giải phóng bộ nhớ
void giaiPhong(BITMAP &img)
{
   if (img.ColorsTable!=NULL)
    {
        delete[] img.ColorsTable;
        img.ColorsTable=NULL;
    }
    if (img.overage!=NULL)
    {
        delete[] img.overage;
        img.overage=NULL;
    }
    if (img.PixelData!=NULL)
    {
        delete[] img.PixelData;
        img.PixelData=NULL;
    }
}

//  Hàm xuất thông tin ảnh ra màn hình để kiểm tra
void Output(BITMAP image)
{
    cout<<"---------------Thong tin hinh anh----------------\n";
    cout<<"/******Header:\n";
    cout<<"signature: "<<image.header.sign1<<image.header.sign2<<endl;
    cout<<"FileSize: "<<image.header.fSize<<endl;
    cout<<"DataOffset: "<<image.header.DataOffset<<endl<<endl;
    cout<<"/******DIB:\n";
    cout<<"DIB size: "<<image.dib.dibSize<<endl;
    cout<<"Width: "<<image.dib.width<<endl;
    cout<<"Height: "<<image.dib.height<<endl;
    cout<<"Planes: "<<image.dib.planes<<endl;
    cout<<"Bits per pixel: "<<image.dib.bpp<<endl;
    cout<<"Compression: "<<image.dib.compression<<endl;
    cout<<"Image Size: "<<image.dib.ImageSize<<endl;
    cout<<"XpixelsPerM: "<<image.dib.XppM<<endl;
    cout<<"YpixelsPerM: "<<image.dib.YppM<<endl;
    cout<<"Colors Used: "<<image.dib.Colors<<endl;
    cout<<"Important Color: "<<image.dib.ImpColor<<endl<<endl;
}

int main(int argc, char *argv[])
{
    //  Viết chương trình theo tham số dòng lệnh

    if (argc==4)        //  có dạng: <tên file CT> -conv <đường dẫn file Inp> <đường dẫn file Outp> 
    {
        if (strcmp(argv[1],"-conv")==0)
        {
            //  Khai báo con trỏ char lưu đường dẫn tên file
            char fin[200];
            strcpy(fin,argv[2]);
            char fout[200];
            strcpy(fout,argv[3]);

            //  Xử lí
            BITMAP img1;
            ReadBmp(fin,img1);      //  Đọc file ảnh 24bpp / 32bpp
            BITMAP img2;
            ConvertBmp(img1,img2);  //  Chuyển sang ảnh 8bpp 
            SaveBmp(fout,img2);     //  Lưu ảnh kết quả xuống đường dẫn file output

            //  Giải phóng bộ nhớ
            giaiPhong(img1);
            giaiPhong(img2);

            return 225;
        }
    }
    else
    {
        if (argc==5)    //  //  có dạng: <tên file CT> -zoom <đường dẫn file Inp> <đường dẫn file Outp> S
        {
            if (strcmp(argv[1],"-zoom")==0)
            {
            //  Khai báo con trỏ char lưu đường dẫn tên file
                char fin[200];
                strcpy(fin,argv[2]);
                char fout[200];
                strcpy(fout,argv[3]);

                //  Chuyển tỉ lệ S từ kiểu xâu thành số nguyên
                int s = atoi(argv[4]);

                //  Xử lí
                BITMAP img1;
                ReadBmp(fin,img1);           //  Đọc file ảnh 32bpp/24bpp/8bpp
                BITMAP img2;
                ZoomOutBmp(img1,img2,s);     //  Thu nhỏ ảnh theo tỉ lệ s
                SaveBmp(fout,img2);          //  Lưu ảnh kết quả xuống đường dẫn file output

                //  Giải phóng bộ nhớ
                giaiPhong(img1);
                giaiPhong(img2);

                return 225;
            }
        }
    }
    return 225;
}
