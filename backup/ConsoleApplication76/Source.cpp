#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <map>
using namespace std;

//const int LEYE  = 0;
//const int REYE  = 1;
const int NOSE  = 2;
const int MOUTH = 3;
const double SCALE = 1;

typedef pair<std::string, cv::Rect> pair_id_rect;

enum faceParts{ face, eye, nose, mouth };

typedef struct CascadeData{
	CascadeData(const cv::Mat& image, 
		std::string name, 
		const cv::Size rectSize, 
		const int rectQuantity, 
		const cv::Scalar color, 
		int cnt, 
		faceParts id)
		:img(image.clone()),
		detector(name), 
		minRectSize(rectSize), 
		minRectQuantity(rectQuantity), 
		rectColor(color), 
		count(cnt), 
		cascadeId(id) 
	{}

	cv::Mat		img;
	std::string detector;
	cv::Size	minRectSize;      //scale
	int			minRectQuantity;
	cv::Scalar	rectColor;
	int			count;
	faceParts cascadeId;
} CascadeData;

template<typename T>
struct Point{
	T x;
	T y;
	Point() : x(-1),y(-1) {}
	Point(T x_, T y_) : x(x_), y(y_) {}
};

class PersonData{
public:
	int id;
	cv::Rect eyeR;
	cv::Rect eyeL;
	cv::Rect nose;
	cv::Rect mouse;
	cv::Mat image;

	//未定義
	int face_color;
	int eyeMany;
	int noseMany;
	int mouseMany;

	PersonData(){
		id = -1;
	}

	PersonData(int id_) : id(id_) {}
	PersonData(int id_, cv::Mat& image_) : id(id_), image(image_.clone()){}
	//以下のコンストラクタは想定しない。
	//PersonData(int ID_, Point eyeR_, Point eyeL_,... )
};

//顔を検出し、その画像をそれぞれ出力する
int find_face(const cv::Mat photo, const std::string cascadeFilename, std::vector<PersonData>& output){
	if(!output.empty()) return -1;

	std::vector<PersonData> temp;

	cv::CascadeClassifier cascade;

	CascadeData cascadeData(photo, cascadeFilename, cv::Size(photo.size().width /10,photo.size().width/10), 1, cv::Scalar(  0,  0,255), 0, face);
	if(!cascade.load(cascadeFilename)) return -1;

	std::vector<cv::Rect> faces;

	cascade.detectMultiScale(photo, faces,
		1.1, cascadeData.minRectQuantity,
		CV_HAAR_SCALE_IMAGE
		,											
		cascadeData.minRectSize);

	int id_ = 0;
	for(cv::Rect rect: faces){
		cv::Mat roi_img(photo, rect);
		PersonData temp_(id_ ,roi_img);
		output.push_back(temp_);

		//test code => ok
		//cv::imshow("Capture", roi_img);
		//cv::imwrite("face_track.jpg",roi_img);
		//cv::waitKey(0);
		++id_;
	}

	//顔画像が無ければ
	if(id_ == 0) return -1;


	return 0;
}

//顔の写真から目・口・鼻を見つけ出す
int find_facePart(std::vector<PersonData>& input, const std::vector<CascadeData> cascades)
{
	if(input.size() == 0){
		std::cout << "inputed vectors size is too small" << endl;
		return -1;
	}

	cv::CascadeClassifier cascade;

	for(int i=0; i < (int)input.size(); i++){
		auto input_ = input[i];
		cv::Mat face_img = input_.image;
		for(CascadeData cascadeData : cascades){
			std::vector<cv::Rect> recogedParts;

			// マルチスケール（顔）探索
			// 画像，出力矩形，縮小スケール，最低矩形数，（フラグ），最小矩形
			cascade.detectMultiScale(face_img, recogedParts,
				1.1,  cascadeData.count,
				CV_HAAR_SCALE_IMAGE
				,
				cv::Size(cascadeData.minRectSize.width, cascadeData.minRectSize.height));

			cout << "test" << endl;
			cout << recogedParts.size() << endl;

			switch (cascadeData.cascadeId)
			{
			case eye:
				if(recogedParts.size() < 2){
					std::cout << "Couldn't find eye" << endl;
					return -1;
				}
				input[i].eyeL = recogedParts[0];
				input[i].eyeR = recogedParts[1];
				break;
			case mouth:
				if(recogedParts.size() == 0){
					std::cout << "Couldn't find mouth" << endl;
					return -1;
				}
				input[i].mouse = recogedParts[0];
				break;
			case nose:
				if(recogedParts.size() == 0){
					std::cout << "Couldn't find nose" << endl;
					return -1;
				}
				input[i].nose = recogedParts[0];
				break;
			default:
				break;
			}
			
			for(const auto r: recogedParts){
				cv::Point leftDown(int((r.x)*SCALE), int((r.y*SCALE)));
				cv::Point  rightUp(int((r.x + r.width)*SCALE), int((r.y + r.height)*SCALE));
				cv::rectangle(input_.image, rightUp, leftDown, cascadeData.rectColor, 3);
			}
		}
	}


	return 0;
}

int main(int argc, char *argv[])
{
	cv::namedWindow("Capture", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
	cv::Mat img = (argc > 1)? cv::imread(argv[1]) : cv::imread("lena.jpg");
	//while(1) {
	const double SCALE = 1;
#if 0

	cv::Mat gray, smallImg(cv::saturate_cast<int>(img.rows/SCALE), cv::saturate_cast<int>(img.cols/SCALE), CV_8UC1);
	// グレースケール画像に変換
	cv::cvtColor(img, gray, CV_BGR2GRAY);
	// 処理時間短縮のために縮小
	cv::resize(gray, smallImg, smallImg.size(), 0, 0, cv::INTER_LINEAR);
	cv::equalizeHist( smallImg, s画像をmallImg);

#endif
	//ADV: face_img に顔画像を保存し、それを解析
	//FIX: for文の順番的に、複数人いる場合、検知不可能
	std::vector<PersonData> faces;

	if(find_face(img, "C:/opencv/data/haarcascades/haarcascade_frontalface_default.xml", faces) == -1) {
		std::cout << "error occored in face_img" << std::endl; 
		return -1;
	}

	std::vector<CascadeData> cascades;
	cascades.push_back(CascadeData(img, "C:/opencv/data/haarcascades/haarcascade_eye.xml",		 cv::Size(1,1), 2, cv::Scalar(  0,255,  0), 0, eye)); //eye	  //blue
	cascades.push_back(CascadeData(img, "C:/opencv/data/haarcascades/haarcascade_mcs_mouth.xml", cv::Size(1,1), 1, cv::Scalar(255,  0,  0), 1, mouth)); //mouth //yellow
	cascades.push_back(CascadeData(img, "C:/opencv/data/haarcascades/haarcascade_mcs_nose.xml",  cv::Size(1,1), 1, cv::Scalar(255,255,255), 1, nose)); //nose  //white

	/*if(find_facePart(faces, cascades) == -1) {
		std::cout << "error occored in face_img" << std::endl; 
		return -1;
	}*/

	//test code => true
	/*for(auto test : faces)
	{
		std::cout << test.id << endl;
		std::cout << "in" << std::endl;
		cv::imshow("Capture", test.image);
		cv::waitKey(0);
	}*/



#if 0
	cv::Mat face_img;

	//temp
	cv::Rect face_rect;

	//検出器を入れておく
	std::vector<CascadeData> cascades;
	cascades.push_back(CascadeData(face_img, "C:/opencv/data/haarcascades/haarcascade_eye.xml",		  cv::Size(4,4), 2, cv::Scalar(  0,255,  0), 2, eye)); //eye	  //blue
	cascades.push_back(CascadeData(face_img, "C:/opencv/data/haarcascades/haarcascade_mcs_mouth.xml", cv::Size(5,5), 1, cv::Scalar(255,  0,  0), 1, mouth)); //mouth //yellow
	cascades.push_back(CascadeData(face_img, "C:/opencv/data/haarcascades/haarcascade_mcs_nose.xml",  cv::Size(5,5), 1, cv::Scalar(255,255,255), 1, nose)); //nose  //white

	//検出器
	cv::CascadeClassifier cascade;

	//目・口・鼻の検出する範囲として、顔を検出して利用する
	std::vector<cv::Rect> faces;
	CascadeData cascadeData = CascadeData(img, "C:/opencv/data/haarcascades/haarcascade_frontalface_default.xml",     cv::Size(img.size().width /10,img.size().width/10), 1, cv::Scalar(  0,  0,255), 0, "face");
	if(!cascade.load("C:/opencv/data/haarcascades/haarcascade_frontalface_default.xml")) return -1;

	cascade.detectMultiScale(img, faces,
		1.1, cascadeData.minRectQuantity,
		CV_HAAR_SCALE_IMAGE
		,											
		cascadeData.minRectSize);

	//顔を矩形で囲む
	for(const auto r: faces){
		cv::Point leftDown(int(r.x*SCALE), int(r.y*SCALE));
		cv::Point  rightUp(int((r.x + r.width)*SCALE), int((r.y + r.height)*SCALE));
		cv::rectangle(img, rightUp, leftDown, cascadeData.rectColor, 3);
	}

	//顔に合わせて、目・口・鼻の検出をする
	for(cv::Rect face_rect:faces){
		face_img = img(face_rect);
		for(CascadeData cascadeData : cascades){
			if(!cascade.load(cascadeData.detector)) return -1;

			std::vector<cv::Rect> recogedParts;

			// マルチスケール（顔）探索
			// 画像，出力矩形，縮小スケール，最低矩形数，（フラグ），最小矩形
			cascade.detectMultiScale(face_img, recogedParts,
				1.1,  cascadeData.count,
				CV_HAAR_SCALE_IMAGE
				,
				cv::Size(face_rect.width / (cascadeData.minRectSize).width, face_rect.height / (cascadeData.minRectSize).height));

			for(const auto r: recogedParts){
				cv::Point leftDown(int((r.x+face_rect.x)*SCALE), int((r.y+face_rect.y)*SCALE));
				cv::Point  rightUp(int((r.x + r.width + face_rect.x)*SCALE), int((r.y + r.height + face_rect.y)*SCALE));
				cv::rectangle(img, rightUp, leftDown, cascadeData.rectColor, 3);
			}
		}
#endif

#if 0
		//ADV: face_img に顔画像を保存し、それを解析
		//FIX: for文の順番的に、複数人いる場合、検知不可能
		cv::Mat face_img = img;
		std::vector<CascadeData> cascades;
		cascades.push_back(CascadeData(img, "C:/opencv/data/haarcascades/haarcascade_frontalface_default.xml",     cv::Size(50,50), 1, cv::Scalar(  0,  0,255))); //face  //red
		cascades.push_back(CascadeData(face_img, "C:/opencv/data/haarcascades/haarcascade_eye.xml",				   cv::Size(30,30), 2, cv::Scalar(  0,255,  0))); //eye	  //blue
		cascades.push_back(CascadeData(face_img, "C:/opencv/data/haarcascades/haarcascade_mcs_mouth.xml",          cv::Size(60,30), 1, cv::Scalar(255,  0,  0))); //mouth //yellow
		cascades.push_back(CascadeData(face_img, "C:/opencv/data/haarcascades/haarcascade_mcs_nose.xml",           cv::Size(30,40), 1, cv::Scalar(255,255,255))); //nose  //white
		for(auto cascadeData : cascades){
			cv::CascadeClassifier cascade;
			if(!cascade.load(cascadeData.detector)) return -1;

			std::vector<cv::Rect> recogedParts;

			// マルチスケール（顔）探索
			// 画像，出力矩形，縮小スケール，最低矩形数，（フラグ），最小矩形
			cascade.detectMultiScale(cascadeData.img, recogedParts,
				1.1, cascadeData.minRectQuantity,
				CV_HAAR_SCALE_IMAGE
				,
				cascadeData.minRectSize);

			for(const auto r: recogedParts){
				cv::Point leftDown(int(r.x*SCALE), int(r.y*SCALE));
				cv::Point  rightUp(int((r.x + r.width)*SCALE), int((r.y + r.height)*SCALE));
				cv::rectangle(img, rightUp, leftDown, cascadeData.rectColor, 3);
			}
#endif

#if 0
			for(auto r:faces) {
				cv::Point center;
				int radius;
				center.x = cv::saturate_cast<int>((r.x + r.width*0.5)*SCALE);
				center.y = cv::saturate_cast<int>((r.y + r.height*0.5)*SCALE);
				radius = cv::saturate_cast<int>((r.width + r.height)*0.25*SCALE);
				cv::circle( img, center, radius, cascadeData.rectColor, 3, 8, 0 );
			}
#endif
			cv::imshow("capture", faces[0].image);
			cv::imwrite("face_track.jpg",img);
			cv::waitKey(0);
			return 0;
		}


#if 0
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
		using namespace std;

		int main(int argc, char *argv[])
		{
			cv::VideoCapture cap(0);

			cv::Size cap_size(640, 480);
			cap.set(CV_CAP_PROP_FRAME_WIDTH, cap_size.width);
			cap.set(CV_CAP_PROP_FRAME_HEIGHT, cap_size.height);
			// カメラがオープンできたかの確認
			if(!cap.isOpened()) return -1;
			// ビデオライタ
			int fps = 60;
			//cv::VideoWriter writer("capture.avi", CV_FOURCC('X','V','I','D'), fps, cap_size);

			cv::namedWindow("Capture", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
			cv::Mat img;   
			while(1) {
				cap >> img;  // キャプチャ
				// 様々な処理
				// ...

				const double SCALE = 1.5;
				cv::Mat gray, smallImg(cv::saturate_cast<int>(img.rows/SCALE), cv::saturate_cast<int>(img.cols/SCALE), CV_8UC1);
				// グレースケール画像に変換
				cv::cvtColor(img, gray, CV_BGR2GRAY);
				// 処理時間短縮のために画像を縮小
				cv::resize(gray, smallImg, smallImg.size(), 0, 0, cv::INTER_LINEAR);
				cv::equalizeHist( smallImg, smallImg);

				// 分類器の読み込み
				// 顔の正面を検出
				std::vector<std::string> cascade_xmls;
				cascade_xmls.push_back("C:/opencv/data/haarcascades/haarcascade_frontalface_alt.xml"); // Haar-like
				cascade_xmls.push_back("C:/opencv/data/haarcascades/haarcascade_eye.xml");
				for(auto cascadeName : cascade_xmls){
					cv::CascadeClassifier cascade;
					if(!cascade.load(cascadeName)) return -1;

					std::vector<cv::Rect> faces;

					// マルチスケール（顔）探索
					// 画像，出力矩形，縮小スケール，最低矩形数，（フラグ），最小矩形
					cascade.detectMultiScale(smallImg, faces,
						1.1, 2,
						CV_HAAR_SCALE_IMAGE
						,
						cv::Size(30, 30));		

					for(auto r:faces) {
						cv::Point center;
						int radius;
						center.x = cv::saturate_cast<int>((r.x + r.width*0.5)*SCALE);
						center.y = cv::saturate_cast<int>((r.y + r.height*0.5)*SCALE);
						radius = cv::saturate_cast<int>((r.width + r.height)*0.25*SCALe);
						cv::circle( img, center, radius, cv::Scalar(80,80,255), 3, 8, 0 );
					}
				}
				cv::imshow("Capture", img);
				if(cv::waitKey(30) >= 0) break;
			}
		}
#endif