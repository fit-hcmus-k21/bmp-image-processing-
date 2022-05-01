#include <iostream>
#include <math.h>
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
    char B;             //  Blue
    char G;             //  Green
    char R;             //  Red
}  PIXEL;

//  Khai báo cấu trúc ảnh BMP

typedef struct 
{
    HEADER header;
    DIB dib;
    char *ColorsTable = NULL;  //  con trỏ tới vùng nhớ lưu thông tin bảng màu (nếu có)
    char *overage = NULL;      //  con trỏ tới vùng nhớ lưu phần dư của DIB (nếu có)
    char *PixelData =   NULL;    //  con trỏ tới vùng nhớ lưu dữ liệu điểm ảnh
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
        image.overage = new char[image.dib.dibSize-40];             //  1. Nhớ giải phóng bộ nhớ
        fin.read((char *) image.overage,image.dib.dibSize-40);
    }

    //  Cấp phát vùng nhớ cho con trỏ đến dữ liệu điểm ảnh
    image.PixelData = new char[image.dib.ImageSize];                //  2. Nhớ giải phóng bộ nhớ     

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
        image.ColorsTable = new char[size];                            //  3. Nhớ giải phóng bộ nhớ
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
    img2.ColorsTable=new char[size];                                //  4.  Nhớ giải phóng bộ nhớ
    //  Tạo thông tin bảng màu cho ảnh 8bpp
    int d=0;
    for (int i=0;i<256;i++)
    {
        img2.ColorsTable[d++] = (char) i;
        img2.ColorsTable[d++] = (char) i;
        img2.ColorsTable[d++] = (char) i;
        d++;
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
    img2.header.fSize = img2.dib.dibSize + sizeof(HEADER) + img2.dib.ImageSize;

    //  Cấp phát vùng nhớ cho dữ liệu điểm ảnh của ảnh đích
    img2.PixelData=new char[img2.dib.ImageSize];                        //  5. Nhớ giải phóng bộ nhớ

    /****  Chuyển dữ liệu điểm ảnh                  ****/

    //  Khai báo con trỏ PixPtr đi qua từng pixel của ảnh nguồn
    char *PixPtr=img1.PixelData;

    //  Chuyển dữ liệu điểm ảnh từ ảnh nguồn (24bpp/32bpp) sang ảnh đích 8bpp
    char B,G,R;

    //  Đối với ảnh 24bit
    if (img1.dib.bpp==24)
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
                img2.PixelData[i*(img2.dib.width*img2.dib.bpp/8+padding2)+j] = (char) (B+G+R)/3;
            }
            //  Bỏ qua padding (bytes)
            PixPtr = PixPtr + padding1;
        }
    }

    //  Đối với ảnh 32bit
    if (img1.dib.bpp==32)
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
                img2.PixelData[i*(img2.dib.width*img2.dib.bpp/8+padding2)+j] = (char) (B+G+R)/3;
                
            }
            //  Bỏ qua padding (bytes)
            PixPtr = PixPtr + padding1;
        }
    }

}

//  Hàm thu nhỏ ảnh màu 32bpp, 24bpp, 8bpp theo tỉ lệ S cho trước
void ZoomOutBmp()
{
    
}

//  Hàm xuất thông tin ảnh ra màn hình để kiểm tra
void Output(BITMAP image)
{
    //  Xuất thông tin hình ảnh ra màn hình để kiểm tra
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

//  Viết chương trình theo tham số dòng lệnh
int main()
{
    BITMAP image1;

    //  Đọc ảnh bmp từ đường dẫn file
    char FileInput[50]="260422_1.bmp";
    ReadBmp(FileInput,image1);

    //  Lưu ảnh bmp xuống file theo đường dẫn
    char FileOutput[50]="output1.bmp";
    SaveBmp(FileOutput,image1);

    //  Chuyển ảnh bmp 24bpp/ 32bpp sang 8bpp
    BITMAP image2;
    ConvertBmp(image1,image2);

    //  Lưu ảnh vừa chuyển xuống file theo đường dẫn để kiểm tra
    char BmpConvert[50]="output2.bmp";
    SaveBmp(BmpConvert,image2);

    //  Xuất thông tin hình ảnh ra màn hình để kiểm tra
    Output(image1);
    Output(image2);

    //  Giải phóng bộ nhớ 
    delete[] image1.overage;
    delete[] image2.overage;
    delete[] image1.ColorsTable;
    delete[] image2.ColorsTable;
    delete[] image1.PixelData;
    delete[] image2.PixelData;
    
    
    return 225;
}
