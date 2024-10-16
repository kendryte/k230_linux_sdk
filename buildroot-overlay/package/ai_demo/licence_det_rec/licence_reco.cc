/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "vi_vo.h"
#include "licence_reco.h"

LicenceReco::LicenceReco(const char *kmodel_file, int dict_size, const int debug_mode)
:dict_size(dict_size), AIBase(kmodel_file,"LicenceReco", debug_mode)
{
    model_name_ = "LicenceReco";
    flag = 0;

    input_width = input_shapes_[0][3];
    input_height = input_shapes_[0][2];

    output = new float[input_width * dict_size / 4];

    ai2d_out_tensor_ = this -> get_input_tensor(0);
}

LicenceReco::~LicenceReco()
{
    // delete[] output;
}

void LicenceReco::pre_process(cv::Mat ori_img)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    std::vector<uint8_t> chw_vec;
    Utils::hwc_to_chw(ori_img, chw_vec);
    Utils::resize({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, ai2d_out_tensor_);
}

void LicenceReco::inference()
{
    // this->set_input_tensor(0, ai2d_out_tensor_);
    this->run();
    this->get_output();
}

void LicenceReco::post_process(vector<unsigned char> &results)
{
    output = p_outputs_[0];

	int size = input_width / 4;

	vector<int> result;
	for (int i = 0; i < size; i++)
	{
		float maxs = -10.f;
		int index = -1;
		for (int j = 0; j < dict_size; j++)
		{
			if (maxs < output[i * dict_size + j])
			{
				index = j;
				maxs = output[i * dict_size + j];
			}
		}
		result.push_back(index);
	}

    // 这里是车牌文字对应的十六进制
    std::vector<unsigned char> vec16dict = {0xb9,0xd2,0xca,0xb9,0xc1,0xec,0xb0,0xc4,0xb8,0xdb,0xcd,0xee,0xbb,0xa6,0xbd,0xf2,0xd3,0xe5,0xbc,0xbd,0xbd,0xfa,0xc3,0xc9,0xc1,0xc9,0xbc,0xaa,0xba,0xda,0xcb,0xd5,0xd5,0xe3,0xbe,0xa9,0xc3,0xf6,0xb8,0xd3,0xc2,0xb3,0xd4,0xa5,0xb6,0xf5,0xcf,0xe6,0xd4,0xc1,0xb9,0xf0,0xc7,0xed,0xb4,0xa8,0xb9,0xf3,0xd4,0xc6,0xb2,0xd8,0xc9,0xc2,0xb8,0xca,0xc7,0xe0,0xc4,0xfe,0xd0,0xc2,0xbe,0xaf,0xd1,0xa7,0x30,0x20,0x31,0x20,0x32,0x20,0x33,0x20,0x34,0x20,0x35,0x20,0x36,0x20,0x37,0x20,0x38,0x20,0x39,0x20,0x41,0x20,0x42,0x20,0x43,0x20,0x44,0x20,0x45,0x20,0x46,0x20,0x47,0x20,0x48,0x20,0x4a,0x20,0x4b,0x20,0x4c,0x20,0x4d,0x20,0x4e,0x20,0x50,0x20,0x51,0x20,0x52,0x20,0x53,0x20,0x54,0x20,0x55,0x20,0x56,0x20,0x57,0x20,0x58,0x20,0x59,0x20,0x5a,0x20,0x5f,0x20,0x2d,0x20};

	for (int i = 0; i < size; i++)
		if (result[i] >= 0 && result[i] != 0 && !(i > 0 && result[i-1] == result[i]))
        {
        	results.push_back(vec16dict[(result[i]-1)*2]);
            results.push_back(vec16dict[(result[i]-1)*2 + 1]);
        }
}

std::vector<size_t> sort_indices(const std::vector<cv::Point2f>& vec) 
{
	std::vector<std::pair<cv::Point2f, size_t>> indexedVec;
	indexedVec.reserve(vec.size());

	// 创建带有索引的副本
	for (size_t i = 0; i < vec.size(); ++i) {
		indexedVec.emplace_back(vec[i], i);
	}

	// 按值对副本进行排序
	std::sort(indexedVec.begin(), indexedVec.end(),
		[](const auto& a, const auto& b) {
		return a.first.x < b.first.x;
	});

	// 提取排序后的索引
	std::vector<size_t> sortedIndices;
	sortedIndices.reserve(vec.size());
	for (const auto& element : indexedVec) {
		sortedIndices.push_back(element.second);
	}

	return sortedIndices;
}

void find_rectangle_vertices(const std::vector<cv::Point2f>& points, cv::Point2f& topLeft, cv::Point2f& topRight, cv::Point2f& bottomRight, cv::Point2f& bottomLeft) 
{
    //先按照x排序,比较左右，再按照y比较上下
	auto sorted_x_id = sort_indices(points);

	if (points[sorted_x_id[0]].y < points[sorted_x_id[1]].y)
	{
		topLeft = points[sorted_x_id[0]];
		bottomLeft = points[sorted_x_id[1]];
	}
	else
	{
		topLeft = points[sorted_x_id[1]];
		bottomLeft = points[sorted_x_id[0]];
	}

	if (points[sorted_x_id[2]].y < points[sorted_x_id[3]].y)
	{
        bottomRight = points[sorted_x_id[3]];
		topRight = points[sorted_x_id[2]];

	}
	else
	{ 
        bottomRight = points[sorted_x_id[2]];
		topRight = points[sorted_x_id[3]];
	}
	
}

void LicenceReco::warppersp(cv::Mat src, cv::Mat& dst, BoxPoint b, std::vector<cv::Point2f>& vtd)
{
    Mat rotation;
    vector<Point> con;
    for(auto i : b.vertices)
        con.push_back(i);

    RotatedRect minrect = minAreaRect(con);
    std::vector<cv::Point2f> vtx(4),vt(4);
    minrect.points(vtx.data());

    find_rectangle_vertices(vtx, vtd[0], vtd[1], vtd[2], vtd[3]);
    
    //w,h tmp_w=dist(p1,p0),tmp_h=dist(p1,p2)
    float tmp_w = cv::norm(vtd[1]-vtd[0]);
    float tmp_h = cv::norm(vtd[2]-vtd[1]);
    float w = std::max(tmp_w,tmp_h);
    float h = std::min(tmp_w,tmp_h);

    vt[0].x = 0;
    vt[0].y = 0;
    vt[1].x = w;//w
    vt[1].y = 0;
    vt[2].x = w;
    vt[2].y = h;
    vt[3].x = 0;
    vt[3].y = h;//h
    rotation = cv::getPerspectiveTransform(vtd, vt);

    warpPerspective(src, dst, rotation, Size(w, h));
}

void paint_ascii(cv::Mat& image,int x_offset,int y_offset,unsigned long offset)
{
    Point p;
	p.x = x_offset;
	p.y = y_offset;
	//存放ascii字膜
	char buff[16];           
	//打开ascii字库文件
	FILE *ASCII;
	if ((ASCII = fopen("Asci0816.zf", "rb")) == NULL){
        printf("Can't open ascii.zf,Please check the path!");
		//getch();
		exit(0);
	}
	fseek(ASCII, offset, SEEK_SET);
	fread(buff, 16, 1, ASCII);
    fclose(ASCII);
	int i, j;
	Point p1 = p;
	for (i = 0; i<16; i++)                  //十六个char
	{
		p.x = x_offset;
		for (j = 0; j < 8; j++)              //一个char八个bit
		{
			p1 = p;
			if (buff[i] & (0x80 >> j))    /*测试当前位是否为1*/
			{
				/*
					由于原本ascii字膜是8*16的，不够大，
					所以原本的一个像素点用4个像素点替换，
					替换后就有16*32个像素点
					ps：感觉这样写代码多余了，但目前暂时只想到了这种方法
				*/
				circle(image, p1, 0, Scalar(0, 0, 255), -1);
				p1.x++;
				circle(image, p1, 0, Scalar(0, 0, 255), -1);
				p1.y++;
				circle(image, p1, 0, Scalar(0, 0, 255), -1);
				p1.x--;
				circle(image, p1, 0, Scalar(0, 0, 255), -1);
			}						
			p.x+=2;            //原来的一个像素点变为四个像素点，所以x和y都应该+2
		}
		p.y+=2;
	}
}

void paint_ascii_video(cv::Mat& image,int x_offset,int y_offset,unsigned long offset)
{
    Point p;
	p.x = x_offset;
	p.y = y_offset;
	//存放ascii字膜
	char buff[16];           
	//打开ascii字库文件
	FILE *ASCII;
	if ((ASCII = fopen("Asci0816.zf", "rb")) == NULL){
        printf("Can't open ascii.zf,Please check the path!");
		//getch();
		exit(0);
	}
	fseek(ASCII, offset, SEEK_SET);
	fread(buff, 16, 1, ASCII);
    fclose(ASCII);
	int i, j;
	Point p1 = p;
	for (i = 0; i<16; i++)                  //十六个char
	{
		p.x = x_offset;
		for (j = 0; j < 8; j++)              //一个char八个bit
		{
			p1 = p;
			if (buff[i] & (0x80 >> j))    /*测试当前位是否为1*/
			{
				/*
					由于原本ascii字膜是8*16的，不够大，
					所以原本的一个像素点用4个像素点替换，
					替换后就有16*32个像素点
					ps：感觉这样写代码多余了，但目前暂时只想到了这种方法
				*/
				circle(image, p1, 0, Scalar(255,153,18), -1);
				p1.x++;
				circle(image, p1, 0, Scalar(255,153,18), -1);
				p1.y++;
				circle(image, p1, 0, Scalar(255,153,18), -1);
				p1.x--;
				circle(image, p1, 0, Scalar(255,153,18), -1);
			}						
			p.x+=2;            //原来的一个像素点变为四个像素点，所以x和y都应该+2
		}
		p.y+=2;
	}
}

void paint_chinese(cv::Mat& image,int x_offset,int y_offset,unsigned long offset)
{
    Point p;
    p.x=x_offset;
    p.y=y_offset;
    FILE *HZK;
    char buff[72];//72个字节，用来存放汉字的
   if((HZK=fopen("HZKf2424.hz","rb"))==NULL){
        printf("Can't open HZKf2424.hz,Please check the path!");
        exit(0);//退出
    }
    fseek(HZK, offset, SEEK_SET);/*将文件指针移动到偏移量的位置*/
    fread(buff, 72, 1, HZK);/*从偏移量的位置读取72个字节，每个汉字占72个字节*/
    fclose(HZK);
    bool mat[24][24];//定义一个新的矩阵存放转置后的文字字膜
    int i,j,k;
    for (i = 0; i<24; i++)                 /*24x24点阵汉字，一共有24行*/
	{
		for (j = 0; j<3; j++)                /*横向有3个字节，循环判断每个字节的*/
			for (k = 0; k<8; k++)              /*每个字节有8位，循环判断每位是否为1*/
				if (buff[i * 3 + j] & (0x80 >> k))    /*测试当前位是否为1*/
				{
					mat[j * 8 + k][i] = true;          /*为1的存入新的字膜中*/
				}
				else {
					mat[j * 8 + k][i] = false;
				}
	}
    for (i = 0; i < 24; i++)
	{
		p.x = x_offset;
		for (j = 0; j < 24; j++)
		{		
			if (mat[i][j])
				circle(image, p, 1, Scalar(255, 0, 0), -1);		  //写(替换)像素点
			p.x++;                                                //右移一个像素点
		}
		p.y++;                                                    //下移一个像素点
	}
}

void paint_chinese_video(cv::Mat& image,int x_offset,int y_offset,unsigned long offset)
{
    Point p;
    p.x=x_offset;
    p.y=y_offset;
    FILE *HZK;
    char buff[72];//72个字节，用来存放汉字的
   if((HZK=fopen("HZKf2424.hz","rb"))==NULL){
        printf("Can't open HZKf2424.hz,Please check the path!");
        exit(0);//退出
    }
    fseek(HZK, offset, SEEK_SET);/*将文件指针移动到偏移量的位置*/
    fread(buff, 72, 1, HZK);/*从偏移量的位置读取72个字节，每个汉字占72个字节*/
    fclose(HZK);
    bool mat[24][24];//定义一个新的矩阵存放转置后的文字字膜
    int i,j,k;
    for (i = 0; i<24; i++)                 /*24x24点阵汉字，一共有24行*/
	{
		for (j = 0; j<3; j++)                /*横向有3个字节，循环判断每个字节的*/
			for (k = 0; k<8; k++)              /*每个字节有8位，循环判断每位是否为1*/
				if (buff[i * 3 + j] & (0x80 >> k))    /*测试当前位是否为1*/
				{
					mat[j * 8 + k][i] = true;          /*为1的存入新的字膜中*/
				}
				else {
					mat[j * 8 + k][i] = false;
				}
	}
    for (i = 0; i < 24; i++)
	{
		p.x = x_offset;
		for (j = 0; j < 24; j++)
		{		
			if (mat[i][j])
				circle(image, p, 1, Scalar(255,153,18), -1);		  //写(替换)像素点
			p.x++;                                                //右移一个像素点
		}
		p.y++;                                                    //下移一个像素点
	}
}

//在图片上添加文本
void LicenceReco::draw_text_img(int x_offset,int y_offset,cv::Mat& image,vector<unsigned char> vec16)
{
    //x和y就是第一个字在图片上的起始坐标
    //通过图片路径获取图片
    unsigned char qh,wh;      //定义区号，位号
    unsigned long offset;           //偏移量

    int text_length=vec16.size();  
    std::vector<unsigned char> hexcode = vec16;

    int x =x_offset,y = y_offset;//x,y:在图片上绘制文字的起始坐标
    for(int m=0;m<text_length;){
        if(hexcode[m]==0x23){
            break;//读到#号时结束
        }
        else if(hexcode[m]>0xaf){
            qh=hexcode[m]-0xaf;//使用的字库里是以汉字啊开头，而不是以汉字符号开头
            wh=hexcode[m+1] - 0xa0;//计算位码
            offset=(94*(qh-1)+(wh-1))*72L;
            paint_chinese(image,x,y,offset);
            /*
            计算在汉字库中的偏移量
            对于每个汉字，使用24*24的点阵来表示的
            一行有三个字节，一共24行，所以需要72个字节来表示
            */
            m=m+2;//一个汉字的机内码占两个字节，
            x+=24;//一个汉字为24*24个像素点，由于是水平放置，所以是向右移动24个像素点
        }
        else{//当读取的字符为ASCII码时
            wh=hexcode[m];
            offset=wh*16l;//计算英文字符的偏移量
            paint_ascii(image,x,y,offset);
            m++;//英文字符在文件里表示只占一个字节，所以往后移一位就行了
            x+=16;
        }
    }
}

void LicenceReco::draw_text_video(float x_offset,float y_offset,cv::Mat& src_img,vector<unsigned char> vec16)
{
    int src_w_osd = src_img.cols;
    int src_h_osd = src_img.rows;
    
    int src_w = 300;
    int src_h = 40;
    int res_w = 450;
    int res_h = 100;

    cv::Mat txt_src(src_h, src_w, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::Mat txt_res;


    //x和y就是第一个字在图片上的起始坐标
    //通过图片路径获取图片
    unsigned char qh,wh;      //定义区号，位号
    unsigned long offset;           //偏移量

    int text_length=vec16.size();  
    std::vector<unsigned char> hexcode = vec16;

    int x = 5;
    int y = 5;//x,y:在图片上绘制文字的起始坐标
    for(int m=0;m<text_length;){
        if(hexcode[m]==0x23){
            break;//读到#号时结束
        }
        else if(hexcode[m]>0xaf){
            qh=hexcode[m]-0xaf;//使用的字库里是以汉字啊开头，而不是以汉字符号开头
            wh=hexcode[m+1] - 0xa0;//计算位码
            offset=(94*(qh-1)+(wh-1))*72L;
            paint_chinese_video(txt_src,x,y,offset);
            /*
            计算在汉字库中的偏移量
            对于每个汉字，使用24*24的点阵来表示的
            一行有三个字节，一共24行，所以需要72个字节来表示
            */
            m=m+2;//一个汉字的机内码占两个字节，
            x+=24;//一个汉字为24*24个像素点，由于是水平放置，所以是向右移动24个像素点
        }
        else{//当读取的字符为ASCII码时
            wh=hexcode[m];
            offset=wh*16l;//计算英文字符的偏移量
            paint_ascii_video(txt_src,x,y,offset);
            m++;//英文字符在文件里表示只占一个字节，所以往后移一位就行了
            x+=16;
        }
    }

    cv::resize(txt_src, txt_res, cv::Size(res_w,res_h), 0, 0, INTER_AREA);

    int txt_x = max(int ((x_offset/SENSOR_WIDTH) * src_w_osd),0);
    int txt_y = max(int ((y_offset/SENSOR_HEIGHT) * src_h_osd),0);//x,y:在图片上绘制文字的起始坐标
 
    if(txt_x+res_w<src_w_osd & txt_y+res_h<src_h_osd)
    {
        cv::Mat text_roi = src_img(cv::Rect(txt_x,txt_y,res_w,res_h));
        txt_res.copyTo(text_roi,txt_res);  //第一个参数目标图，第二个参数原图
    }
}