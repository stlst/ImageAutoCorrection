#include <highgui.h>
#include<string>
#include <cv.h>
#include<list>
#include <cxcore.h>  //人脸识别的一个库文件
using namespace std;
#define PI 3.1415926
#define AA 1
#define BB 2
#define AB 3
#define BA 4
bool success = false;

inline double point_distance(CvPoint one,CvPoint two){
	return sqrt((double)(one.x - two.x)*(one.x - two.x) + (one.y - two.y) * (one.y - two.y));
}
inline double point_distancef(CvPoint2D32f one,CvPoint2D32f two){
	return sqrt((double)(one.x - two.x)*(one.x - two.x) + (one.y - two.y) * (one.y - two.y));
}
class Line{
public:
	CvPoint a;
	CvPoint b;
	double angle;
	Line(){
		a.x = 0;
		a.y = 0;
		b.x = 0;
		b.y = 0;
		angle = 0.0;
	};
	Line(CvPoint point1,CvPoint point2){
		a = point1;
		b = point2;
		if(abs(a.x - b.x) < 3)
			angle = 90;
		else{
			double slope = (a.y - b.y)/(double)(a.x-b.x);
			angle = atan(slope) /PI *180;
		}
		if(angle < 0)
			angle += 360;
		//printf("angle %lf\n",angle);
	}
	void UpdateAngle(){
		if(abs(a.x - b.x) < 3)
			angle = 90;
		else{
			double slope = (a.y - b.y)/(double)(a.x-b.x);
			angle = atan(slope) /PI *180;
		}
		if(angle < 0)
			angle += 360;
	}
	int IsNear(Line one, Line two){
		if(point_distance(one.a,two.a)<4 )
			return AA;
		else if(point_distance(one.b,two.a)<4)
			return BA;
		else if(point_distance(one.b,two.b)<4)
			return BB;
		else if(point_distance(one.a,two.b)<4)
			return AB;
		else 
			return 0;
	}
	double FindCrossAngle(Line other){
		double max,min;
		if(other.angle > this->angle){
			max = other.angle;
			min = this->angle;
		}else{
			min = other.angle;
			max = this->angle;
		}
		double temp = max - min;
		temp = temp > 180 ? temp - 180 : temp;
		return temp;
	}
};



inline void drawcross(CvArr* img,CvPoint2D32f pt)
{
    const int radius=3;
    int ptx=cvRound(pt.x);
    int pty=cvRound(pt.y);
    int ls=ptx-radius;
    int re=ptx+radius;
    int us=pty-radius;
    int de=pty+radius;
    cvLine(img,cvPoint(ls,pty),cvPoint(re,pty),CV_RGB(255,255,255),1,0);
    cvLine(img,cvPoint(ptx,us),cvPoint(ptx,de),CV_RGB(255,255,255),1,0);
}

void sort_points(CvPoint2D32f* SrcPoint, CvSize sz){
	CvPoint2D32f point[4];
	double distance = 1000000000;
	int min_index,max_index;
	for(int i = 0;i<4;i++){//找出左上角的点
		double temp = SrcPoint[i].x * SrcPoint[i].x + SrcPoint[i].y * SrcPoint[i].y;
		if( temp < distance){
			point[0] = SrcPoint[i];
			distance = temp;
			min_index = i;
		}
	}
	int max = 0;
	for(int i = 0;i<4;i++){
		if(i != min_index){
			double temp_distance = (SrcPoint[i].x - point[0].x) * (SrcPoint[i].x - point[0].x) + 
				(SrcPoint[i].y - point[0].y) * (SrcPoint[i].y - point[0].y);
			if(temp_distance > max){
				max = temp_distance;
				point[3].x = SrcPoint[i].x;
				point[3].y = SrcPoint[i].y;
				max_index = i;
			}
		}
	}

	CvPoint2D32f mid;
	mid.x = (point[0].x+point[3].x)/2;
	mid.y = (point[0].y+point[3].y)/2;

	CvPoint2D32f a,b;
	int i = 0;
	while(i == max_index || i == min_index){
		i++;
	}
	a.x = point[0].x - SrcPoint[i].x;
	a.y = point[0].y - SrcPoint[i].y;
	b.x = mid.x - SrcPoint[i].x;
	b.y = mid.y - SrcPoint[i].y;
	if(a.x *b.y - a.y *b.x <0){
		point[2] = SrcPoint[i];
		for(int j = 0;j<4;j++){
		if(j != min_index && j != max_index && j != i)
			point[1] = SrcPoint[j];
		}
	}else{
		point[1] = SrcPoint[i];
		for(int j = 0;j<4;j++){
		if(j != min_index && j != max_index && j != i)
			point[2] = SrcPoint[j];
		}
	}
	
	/*
	long min = 10000000000000;
	int index_1,index_2;
	for(int i = 0;i<4;i++){
		if(i != min_index && i != max_index){
			double temp_distance = SrcPoint[i].x * SrcPoint[i].x + (SrcPoint[i].y - sz.width) * (SrcPoint[i].y - sz.width);
			if(temp_distance < min){
				min = temp_distance;
				point[1] = SrcPoint[i];
				index_1 = i;
			}
		}
	}
	for(int i = 0;i<4;i++){
		if(i != min_index && i != max_index && i != index_1)
			point[2] = SrcPoint[i];
	}
	*/

	for(int i = 0;i<4;i++){
		SrcPoint[i] = point[i];
	}
	
}

list<Line> find_lines(IplImage* dst){
	
	CvSeq * lines = 0;
	list<Line> select_line; 
	CvMemStorage * storage = cvCreateMemStorage();
	lines = cvHoughLines2(dst,storage,CV_HOUGH_PROBABILISTIC,1,CV_PI/180,100,70,300);
	cvSetZero(dst);
	//IplImage *Dst = cvCloneImage(dst);
	list<Line>::iterator it;
	if(lines->total != 0){
		CvPoint* line = (CvPoint*)cvGetSeqElem (lines, 0); 
		Line pair(line[0],line[1]);
		select_line.push_back(pair);
	}
	//cvLine (dst, line[0], line[1], CV_RGB(255,255,255),8,0); 
	
	for (int i = 1; i < lines->total; i++){  
		CvPoint* line = (CvPoint*)cvGetSeqElem (lines, i);  
		
		///////////////////////////////////////////////////
		it = select_line.begin();
		Line temp(line[0],line[1]);
		//printf("line %d (%d %d)\n",i,line[0].x,line[0].y);
		while(it != select_line.end() ){
			int position = temp.IsNear(temp,*it);
			if(abs(temp.angle - it->angle) <= 5 && position){
				if(position == AA){//20140426
					it->a = it->b;
					it->b = temp.b;
					it->UpdateAngle();
				}else if(position == AB){
					it->a = it->a;
					it->b = temp.b;
					it->UpdateAngle();
				}else if(position == BA){
					it->a = it->b;
					it->b = temp.a;
					it->UpdateAngle();
				}else if(position == BB){
					it->a = it->a;
					it->b = temp.a;
					it->UpdateAngle();
				}
				printf("delete\n");
				break;
			}
			else{
				//select_line.push_back(temp);
			}
			it++;
			//printf("%d\n",i);
		}
		if(it == select_line.end())
			select_line.push_back(temp);
	}
	
	return select_line;
}
void sort_lines(list<Line> &select_line,list<Line> &target_line){
	if(select_line.empty()){success = false;return;}
	list<Line>::iterator it = select_line.begin();
	
	int * arr = new int[select_line.size()];
	for(int i = 0;i<select_line.size();i++){
		arr[i] = 0;
	}
	int count = 0;
	int count1 = 0;
	while(it != select_line.end()){ 
		list<Line>::iterator ita = ++it;
		it--;
		count1 = count + 1;
		while(ita != select_line.end()){
			double temp = it->FindCrossAngle(*ita);
			printf("%lf\n",temp);
			if(temp < 8 || temp > 173){}
			else if((temp > 15 && temp < 75)||(temp > 105 && temp < 165)){//不符合
				arr[count1] += 2;
				arr[count] += 2;
				printf("不符合\n");
			}
			ita++;
			count1++;
		}
		it++;
		count++;
	}
	it = select_line.begin();
	int size = select_line.size();
	for(int i = 0;i<size;i++){
		printf("%d\n",arr[i]);
		if(arr[i] > size / 2 + 1){
		}
		else{
			target_line.push_back(*it);
		}
		it++;
	}
	printf("line count %d\n",target_line.size());
}

void longer(IplImage* dst,list<Line> tar_line){
	list<Line>::iterator it = tar_line.begin();
	while(it != tar_line.end()){
	}
}

//Canny:Implements Canny algorithm for edge detection.
int main( int argc, char** argv )
{
	IplImage* res = NULL;
	IplImage* dst = NULL;
	CvSize sz;  

	//载入图像，转换为灰度图
	res = cvLoadImage( argv[1]);  
	//为canny边缘图像申请空间，1表示单通道灰度图
	
	if(!res){
		printf("不能打开该图片！\n");
		exit(0);
	}
	
	double scale;
	if(res->width > res->height)
		scale = res->width / 600.0;
	else
		scale = res->height / 600.0;
	sz.width = res->width / scale;  
	sz.height = res->height / scale;

	printf("scale factor is %lf \n",scale);

	IplImage* temp = cvCreateImage(cvGetSize(res), res->depth, res->nChannels); 
	if(scale >= 1){
		cvSmooth(res,temp,CV_GAUSSIAN,(int)scale*5,(int)scale*5);//先进行高斯模糊，再进行缩小
	}else{
		cvSmooth(res,temp,CV_GAUSSIAN,1,1);
	}

	


	IplImage* srcRGB = cvCreateImage(sz,res->depth,res->nChannels); 
	IplImage* src = cvCreateImage(sz,IPL_DEPTH_8U,1); 
	cvResize(temp,srcRGB,CV_INTER_CUBIC);
	
	cvCvtColor(srcRGB,src,CV_RGB2GRAY);  
	
	IplImage *color_dst = cvCloneImage(srcRGB);cvSetZero(color_dst); 
	dst = cvCreateImage( cvGetSize( src ), IPL_DEPTH_8U, 1 );
	cvCanny(src,dst, 150, 300, 3 );//边缘检测
	cvCvtColor(dst,color_dst,CV_GRAY2RGB);

	//cvNamedWindow("src");
	//cvShowImage("src",src);
	list<Line> select_line = find_lines(dst);
	printf("line count %d\n",select_line.size());
	list<Line> terget_line;
	sort_lines(select_line,terget_line);
	list<Line>::iterator it = terget_line.begin();
	while(it != terget_line.end()){
		cvLine (dst, it->a, it->b, CV_RGB(255,255,255),8,0);
		it++;
	}
	cvErode(dst,dst,NULL,2);
	//cvNamedWindow("dst");
	//cvShowImage("dst",dst);

	// Create temporary images required by cvGoodFeaturesToTrack  

    IplImage* img_temp = cvCreateImage(cvGetSize(src), 32, 1);  
    IplImage* img_eigen = cvCreateImage(cvGetSize(src), 32, 1);
	

	int cornerCount=30;
	//现在能准确检测出四个角点
	CvPoint2D32f SrcPoint[30];
	CvPoint2D32f DstPoint[30];
	cvGoodFeaturesToTrack(dst,img_eigen,img_temp,SrcPoint,&cornerCount,0.1,50,NULL,19,0,0.04);//注意boxSize的大小！

	if(cornerCount == 4){
		sort_points(SrcPoint,sz);//让找出来的点按左上，右上，左下，右下排好
	}else{//有可能少于4，有可能大于四
		//choose_points(SrcPoint);
		printf("cannot sort points!\n");
	}
	
    for(int i=0;i<cornerCount;i++)
    {
		SrcPoint[i].x *= scale;
		SrcPoint[i].y *= scale;
        //drawcross(dst,SrcPoint[i]);
		//printf("%lf , %lf\n",SrcPoint[i].x,SrcPoint[i].y);
    }

	//IplImage* change_img = cvCreateImage(sz,res->depth,res->nChannels);
	//change_img = cvCloneImage( src );  //制作图像的完整拷贝  
	//change_img ->origin = src ->origin;
	//cvZero(change_img);
	double wid = point_distancef(SrcPoint[0],SrcPoint[1]);
	double hei = point_distancef(SrcPoint[0],SrcPoint[2]);
	CvSize tar;
	if(wid < hei){
		tar.width = 2970;
		tar.height = 2100;
	}else{
		tar.height = 2970;
		tar.width = 2100;
	}
	IplImage* tarImage = cvCreateImage(tar, res->depth, res->nChannels); 

	tarImage->origin = res->origin;
	cvZero(tarImage);
	DstPoint[0].x = 0;
	DstPoint[0].y = 0;
	DstPoint[2].x = tar.width - 1;
	DstPoint[2].y = 0;

	DstPoint[1].x = 0;
	DstPoint[1].y = tar.height - 1;

	DstPoint[3].x = tar.width - 1;
	DstPoint[3].y = tar.height - 1;
	if(cornerCount == 4){
		CvMat *mat = cvCreateMat( 3, 3, CV_32FC1 ); 
		cvGetPerspectiveTransform(SrcPoint,DstPoint,mat );  //取得透视变换矩阵
		cvWarpPerspective(res,tarImage,mat,CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS,cvScalarAll(0)); 
	}
	printf("Detected Points : %d\n", cornerCount);
	string file = argv[1];
	string pre = "test-erode-" + file;
	const char * filename = pre.c_str();
	cvSaveImage(filename,dst);

	pre = "result-" + file;
	const char * filename2 = pre.c_str();
	cvSaveImage(filename2,tarImage);
	//cvNamedWindow ("src");  


	if(cornerCount == 4){
		//cvNamedWindow( "change_img", 1 );
		//cvShowImage( "change_img", tarImage );
	}
	//cvShowImage("src",src);
	
	cvWaitKey();
	cvReleaseImage(&temp);
	cvReleaseImage( &src );
	cvReleaseImage( &dst );
	cvReleaseImage( &img_temp);
	cvReleaseImage( &img_eigen);
	cvReleaseImage( &res);
	cvDestroyAllWindows();
	return 0;

}