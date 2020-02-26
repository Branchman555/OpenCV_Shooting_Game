#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <math.h>
#include <string> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include<Windows.h>
//#include<MMSystem.h>
//#pragma comment(lib,"Winmm.lib")
#define TARGET_CEN_X	365
#define TARGET_CEN_Y	359
#define	HEX_LAR			1600
using namespace cv;
using namespace std;

Point center_replace(0, 0);
Point center_return(0, 0);
Point mid_target(640, 360);
//Mat text_shot = imread("text.png"); Mat(1280, 720, CV_8UC3, Scalar(255, 255, 255));
Mat text_shot = Mat(640, 400, CV_8UC3, Scalar(0, 0, 0));
Mat main_mat = Mat(720, 1280, CV_8UC3, Scalar(255, 255, 255));
Mat target_mat = imread("test2.png");
Mat temp = target_mat(Rect(800, 40, 400, 640));

int score_num = 1;
int score_all = 0;
int zero_weapon = 0;
int zero_weapon_cnt = 0;
int miss = 0;
int status_count = -1;

typedef struct find_Hex {
	vector<Point> corner;
}find_Hex;

/*
void soundPlay_non() {
	sndPlaySoundA("C:\\Users\\J\\Desktop\\std\\imvision\\termp\\non.wav", SND_ASYNC);
}

void soundPlay_shot() {
	sndPlaySoundA("C:\\Users\\J\\Desktop\\std\\imvision\\termp\\shot.wav", SND_ASYNC);
}
*/

find_Hex find_figure(Mat img_input)
{
	Mat img_hsv;
	Mat img_gray;
	vector<vector<Point> > contours;
	vector<Point2f> approx;
	vector<Point> Hex;
	find_Hex Hex_return;
	int size;

	int range_count = 0;

	Scalar red(0, 0, 255);
	Scalar blue(255, 0, 0);
	Scalar yellow(0, 255, 255);

	Scalar magenta(255, 0, 255);

	Mat rgb_color = Mat(1, 1, CV_8UC3, red);
	Mat hsv_color;

	cvtColor(rgb_color, hsv_color, COLOR_BGR2HSV);

	int hue = (int)hsv_color.at<Vec3b>(0, 0)[0];
	int saturation = (int)hsv_color.at<Vec3b>(0, 0)[1];
	int value = (int)hsv_color.at<Vec3b>(0, 0)[2];

	int low_hue = hue - 10;
	int high_hue = hue + 10;

	int low_hue1 = 0, low_hue2 = 0;
	int high_hue1 = 0, high_hue2 = 0;

	Point A(-1, -1);
	vector<Point> B(5, A);
	find_Hex error_Hex;
	error_Hex.corner = B;

	if (low_hue < 10) {
		range_count = 2;

		high_hue1 = 180;
		low_hue1 = low_hue + 180;
		high_hue2 = high_hue;
		low_hue2 = 0;
	}
	else if (high_hue > 170) {
		range_count = 2;

		high_hue1 = low_hue;
		low_hue1 = 180;
		high_hue2 = high_hue - 180;
		low_hue2 = 0;
	}
	else {
		range_count = 1;

		low_hue1 = low_hue;
		high_hue1 = high_hue;
	}

	cvtColor(img_input, img_hsv, COLOR_BGR2HSV);

	Mat img_mask1, img_mask2;
	inRange(img_hsv, Scalar(low_hue1, 50, 50), Scalar(high_hue1, 255, 255), img_mask1);
	if (range_count == 2) {
		inRange(img_hsv, Scalar(low_hue2, 50, 50), Scalar(high_hue2, 255, 255), img_mask2);
		img_mask1 |= img_mask2;
	}

	erode(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	dilate(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	dilate(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	erode(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	Mat img_labels, stats, centroids;
	int numOfLables = connectedComponentsWithStats(img_mask1, img_labels,
		stats, centroids, 8, CV_32S);

	int max = -1, idx = 0;
	for (int j = 1; j < numOfLables; j++) {
		int area = stats.at<int>(j, CC_STAT_AREA);
		if (max < area)
		{
			max = area;
			idx = j;
		}
	}

	int left = stats.at<int>(idx, CC_STAT_LEFT);
	int top = stats.at<int>(idx, CC_STAT_TOP);
	int width = stats.at<int>(idx, CC_STAT_WIDTH);
	int height = stats.at<int>(idx, CC_STAT_HEIGHT);

	//cout << left << endl << top << endl << width << endl << height << endl;
	if (left < 10 || top < 10 || left + width + 10> 1280 || top + height + 10 > 720) {
		//soundPlay_non();
		return error_Hex;
	}

	Mat img_findfigure(img_input, Rect(Point(left - 10, top - 10), Point(left + width + 10, top + height + 10)));
	int col = img_input.cols;
	int row = img_input.rows;
	Mat img_black(Size(col, row), CV_8UC3, Scalar(255, 255, 255));
	Mat roi(img_black, Rect(Point(left - 10, top - 10), Point(left + width + 10, top + height + 10)));
	img_findfigure.copyTo(roi);

	cvtColor(img_black, img_gray, COLOR_RGB2GRAY);
	threshold(img_gray, img_gray, 125, 255, cv::THRESH_BINARY);
	//imshow("result", img_gray);
	findContours(img_gray, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

	for (size_t cnt = 0; cnt < contours.size(); cnt++) {
		approxPolyDP(Mat(contours[cnt]), approx, arcLength(Mat(contours[cnt]), true) * 0.02, true);

		size = approx.size();
		//cout << contours.size() << endl;
		if (size == 6) {
			Hex = { approx[0],approx[1],approx[2],approx[3] ,approx[4] ,approx[5] };
			Hex_return.corner = Hex;
			return Hex_return;
		}

	}
	//soundPlay_non();
	return error_Hex;
}

void game_play(int event, int x, int y, int flags, void* set_image) {
	Mat* pmat = (Mat*)set_image;
	Mat target = Mat(*pmat);

	//Mat text_reset = imread("text.png");
	Mat text_reset = Mat(640, 400, CV_8UC3, Scalar(0, 0, 0));
	Mat text_zero = text_reset.clone();


	Point center_replace_reset(0, 0);

	find_Hex img_Hex;

	unsigned int score;
    int score_range;
	char score_string[20];
	char score_final[20];
	String text_info;
	double degrees = 0;

	

	if (event == CV_EVENT_RBUTTONDOWN) {
		if (status_count == 0) {
			status_count = 2;
		}
		else if (status_count == 1)
		{
			zero_weapon = 1;
			zero_weapon_cnt = 0;
			score_num = 0;
			text_info = "ZERO THE WEAPON";
			target_mat = imread("test2.png");
			temp = target_mat(Rect(800, 40, 400, 640));

			putText(text_zero, text_info, Point(10, 40), 2.5, 1.2, Scalar::all(255));
			//imshow("infoBoard", text_zero);
			text_zero.copyTo(temp);

			text_shot = text_reset;
		}
		
	}

	if (event == CV_EVENT_LBUTTONDOWN) {
		if (status_count == -1) {
			status_count = 0;
			//system("nohup omxplayer first_sound.mp3 &");
			system("nohup mpg321 first_sound.mp3 &");
			//usleep(500);
		}
		else if (status_count == 0) {
			status_count = 1;
		}
		else if (status_count == 1) {
			printf("x:%d, y:%d\n", x, y);
			img_Hex = find_figure(target);
			if (img_Hex.corner[0] == Point(-1, -1)) {
				system("nohup mpg321 Silenced.mp3 &");
				return;
			}
			line(target, img_Hex.corner[0], img_Hex.corner[img_Hex.corner.size() - 1], Scalar(0, 255, 0), 3);
			if (fabs(contourArea(Mat(img_Hex.corner))) > HEX_LAR) {
				for (int k = 0; k < img_Hex.corner.size() - 1; k++) {
					line(target, img_Hex.corner[k], img_Hex.corner[k + 1], Scalar(0, 255, 0), 3);
				}
				center_return = (img_Hex.corner[0] + img_Hex.corner[3]) / 2;
				circle(target, center_return, 3, Scalar(0, 255, 0));
				//imshow("shot", target);
			}

			//Error_Check
			if (center_return == Point(0, 0)) {
				system("nohup mpg321 Silenced.mp3 &");
				return;
			}

			//if (fabs(contourArea(Mat(img_Hex.corner))) > 5000) center_return = (img_Hex.corner[0] + img_Hex.corner[3]) / 2;


			if (zero_weapon) {
				center_replace = center_replace + center_return;
				system("nohup mpg321 gunfire2.mp3 &");
				cout << center_replace << endl;
				sprintf(score_string, "ZP-Shot: %d", zero_weapon_cnt);
				putText(text_zero, score_string, Point(10, 40 + (zero_weapon_cnt) * 40), 2.5, 1.2, Scalar::all(255));
				text_zero.copyTo(temp);
				zero_weapon_cnt++;

				if (zero_weapon_cnt == 5) {
					zero_weapon = 0;
					score_num = 1;
					mid_target = center_replace / zero_weapon_cnt;
					zero_weapon_cnt = 0;

					cout << "replace mid: " << mid_target << endl;
					center_replace = center_replace_reset;
					//imshow("infoBoard", text_reset);
					text_reset.copyTo(temp);
				}
			}
			else {
				if (score_num == 1) {
					text_info = "SHOOTING MODE";
					target_mat = imread("test2.png");
					temp = target_mat(Rect(800, 40, 400, 640));
					putText(text_shot, text_info, Point(10, 40), 2.5, 1.2, Scalar::all(255));
					//imshow("infoBoard", text_shot);
					text_shot.copyTo(temp);
					score_num++;
				}
				else if (score_num > 1) {
					//cout << center_return.x << endl<< center_return.y<<endl;
					//cout << img_Hex.corner[0].x << endl << img_Hex.corner[0].y << endl;
					system("nohup mpg321 gunfire2.mp3 &");
					double range = sqrt(pow((center_return.x - img_Hex.corner[0].x),2) + pow((center_return.y - img_Hex.corner[0].y),2));
					double score = sqrt(pow((center_return.x - mid_target.x),2) + pow((center_return.y - mid_target.y),2));
					degrees = 310 / range;
					printf("range:%lf\n", range);

					int score_range;
					cout << "score: " << score << endl;
					cout << range / 6 << endl << ((range * 2) / 6) << endl << ((range * 3) / 6) << endl << ((range * 4) / 6) << endl << ((range * 5) / 6) << endl;
					if (score >= 0 && score < (range / 6)) {
						score_range = 10;
						//circle(target_mat, Point(TARGET_CEN_X + 0 * (mid_target.x - center_return.x), TARGET_CEN_Y + 0 * (mid_target.y - center_return.y), 5, Scalar(255, 255, 255), 3);
					}
					else if (score >= (range / 6) && score < ((range * 2) / 6)){
						score_range = 8;
						//circle(target_mat, Point(TARGET_CEN_X + 99 * cos(getRadian(degrees)), TARGET_CEN_Y + 99 * sin(getRadian(degrees))), 5, Scalar(255, 255, 255), 3);
					}
					else if (score >= ((range * 2) / 6) && score < ((range * 3) / 6)){
						score_range = 6;
						//circle(target_mat, Point(TARGET_CEN_X + 143 * cos(getRadian(degrees)), TARGET_CEN_Y + 143 * sin(getRadian(degrees))), 5, Scalar(255, 255, 255), 3);
					}
					else if (score >= ((range * 3) / 6) && score < ((range * 4) / 6)){
						score_range = 4;
						//circle(target_mat, Point(TARGET_CEN_X + 242 * cos(getRadian(degrees)), TARGET_CEN_Y + 242 * sin(getRadian(degrees))), 5, Scalar(255, 255, 255), 3);
					}
					else if (score >= ((range * 4) / 6) && score < ((range * 5) / 6)){
						score_range = 2;
						//circle(target_mat, Point(TARGET_CEN_X + 306 * cos(getRadian(degrees)), TARGET_CEN_Y + 306 * sin(getRadian(degrees))), 5, Scalar(255, 255, 255), 3);
					}
					else 
						score_range = 0; {
				
					}
					circle(target_mat, Point((int)(TARGET_CEN_X + degrees * (mid_target.x - center_return.x)), (int)(TARGET_CEN_Y + degrees * (mid_target.y - center_return.y))), 5, Scalar(255, 255, 255), 3);
					//circle(target_mat, Point(TARGET_CEN_X, TARGET_CEN_Y), 5, Scalar(255, 255, 255), 3);
					printf("CX:%d,CY:%d\n", (int)(TARGET_CEN_X + degrees * (mid_target.x - center_return.x)), (int)(TARGET_CEN_Y + degrees * (mid_target.y - center_return.y)));
					cout << score_range << endl;
					score_all = score_range + score_all;

					sprintf(score_string, "SCORE%d: %d", score_num - 1, score_range);
					putText(text_shot, score_string, Point(10, 40 + (score_num - 1) * 40), 2.5, 1.2, Scalar::all(255));
					//imshow("infoBoard", text_shot);
					text_shot.copyTo(temp);
					if (score_num == 11) {
						String yc = "!!Your Final Score!!";
						sprintf(score_final, "%d/100", score_all);
						putText(text_shot, yc, Point(10, 480), 2.5, 1.2, Scalar::all(255));
						putText(text_shot, score_final, Point(75, 550), 2.5, 2, Scalar(0, 255, 0), 3, 8);
						//imshow("infoBoard", text_shot);
						text_shot.copyTo(temp);
					}

					score_num++;
					if (score_num == 12) {
						score_num = 1;
						text_shot = text_reset;
						score_all = 0;
					}
				}
			}
		}
		else if (status_count == 2) {
			status_count = 0;
		}
		
	}
}

int main()
{
	VideoCapture cap1("first_mov.mp4");
	VideoCapture cap(0);
	//Mat cap = imread("test.png");
	Mat frame;
	Mat frame_vrt;
	Mat game_target;

	Mat first_mat = imread("first_phase.png");
	Mat second_mat = imread("second_phase.png");
	namedWindow("shotWindow", 1);
	namedWindow("MainWindow", 1);
	//namedWindow("infoBoard", 1);
	setMouseCallback("MainWindow", game_play, (void*)&game_target);
	while (1) {
		if (status_count == 0) {
			cap1 >> frame;
			if (frame.empty())
			{
				break;
			}
			//resize(frame, frame_vrt, Size(1920, 1080), 0, 0, CV_INTER_LINEAR);
			//imshow("MainWindow", frame_vrt);
			imshow("MainWindow", frame);
		}
		if (waitKey(20) == 27) {}
	}
	while (1) {
		if (status_count == 0) {
			//resize(first_mat, frame_vrt, Size(1920, 1080), 0, 0, CV_INTER_LINEAR);
			//imshow("MainWindow", frame_vrt);
			imshow("MainWindow", first_mat);
		}
		else if (status_count == 1) {
			cap >> frame;
			//frame = cap;
			resize(frame, game_target, Size(1280, 720), 0, 0, CV_INTER_LINEAR);
			//resize(target_mat, frame_vrt, Size(1920, 1080), 0, 0, CV_INTER_LINEAR);
			//imshow("MainWindow", frame_vrt);
			//setMouseCallback("shotWindow", game_play, (void*)& game_target);
			//setMouseCallback("MainWindow", game_play, (void*)&game_target);

			if (game_target.empty()) break;
			
			//imshow("MainWindow", frame_vrt);

			circle(game_target, mid_target, 2, Scalar(255, 0, 0), 3);
			imshow("shotWindow", game_target);
			resize(target_mat, frame_vrt, Size(1920, 1080), 0, 0, CV_INTER_LINEAR);
			imshow("MainWindow", frame_vrt);
			//imshow("MainWindow", frame_vrt);
		}
		else if (status_count == 2) {
			imshow("MainWindow", second_mat);
		}
		if (waitKey(30) >= 0){}// break;
	}

	return 0;
}
