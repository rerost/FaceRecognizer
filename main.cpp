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
	const double SCALE = 1;

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

	cv::imshow("capture", faces[0].image);
	cv::imwrite("face_track.jpg",img);
	cv::waitKey(0);
	return 0;
}
