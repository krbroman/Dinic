#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <opencv2/opencv.hpp>

static const int maxn{ 10803 };
static const int inf{ 1000000000 };

static int n{}, c[maxn][maxn]{}, f[maxn][maxn]{}, s{}, t{}, d[maxn]{}, ptr[maxn]{}, q[maxn]{}, rn[maxn][maxn]{}, mcS[maxn]{}, mcT[maxn]{};

static int rowsImg{}, colsImg{};

bool bfs();
int dfs(int v, int u);
int dinic();
void RemNet();
void SearchNear(int vert);
void MinCut();

std::array<int, 2> decoder(int num, int cols);

int getMaxFlow(const std::string graphAddress);

void getImage(std::string imageAddress, const std::vector<std::array<int, 2>>& background, const std::vector<std::array<int, 2>>& object);

void setBackgroundAndObject(std::vector<std::array<int, 2>>& background, std::vector<std::array<int, 2>>& object);

bool checkAffiliation(const std::vector<std::array<int, 2>>& multiplicity, int i, int j);

double countMetricsOne(const std::string nameImageReuslt, const std::string nameImageSegment);

double countMetricsJaccard(const std::string nameImageReuslt, const std::string nameImageSegment);


static bool eightEdge{ true };
static double delta{ 5 };
static double lambda{ 20 };

int main() {
	//const std::string nameImage{ "fullmoon-gr-100" };

	std::cout << "Eight edges: y/n" << std::endl;
	char eE;
	std::cin >> eE;

	while (true)
	{
		if (eE == 'y')
		{
			eightEdge = true;
			break;
		}
		if (eE == 'n')
		{
			eightEdge = false;
			break;
		}
		else std::cout << "Insert correct symbol: ";
		std::cin >> eE;
	}

	double deltaInput;

	std::cout << "Select delta: (0,100)" << std::endl;
	while (true)
	{
		std::cin >> deltaInput;
		if (deltaInput > 0 && deltaInput <= 100)
		{
			delta = deltaInput;
			break;
		}
		else std::cout << "Insert correct value: ";
	}

	double lambdaInput;

	std::cout << "Select lambda: (0,1000)" << std::endl;
	while (true)
	{
		std::cin >> lambdaInput;
		if (lambdaInput > 0 && lambdaInput <= 1000)
		{
			lambda = lambdaInput;
			break;
		}
		else std::cout << "Insert correct value: ";
	}


	const std::string nameImage{ "banana1-gr-100" };

	std::vector<std::array<int, 2>> background{};
	std::vector<std::array<int, 2>> object{};

	setBackgroundAndObject(background, object);

	getImage(nameImage, background, object);

	int maxFlow{ getMaxFlow(nameImage + ".txt") };

	std::cout << maxFlow << std::endl;

	RemNet();
	MinCut();

	cv::Mat A{ cv::imread(nameImage + ".jpg", cv::IMREAD_GRAYSCALE) };
	for (int i = 1; i < n - 1; ++i) {
		std::array<int, 2> xy = decoder(i, colsImg);

		if (mcS[i] == 1)
			A.at<uchar>(xy[0], xy[1]) = 255;
		else
			A.at<uchar>(xy[0], xy[1]) = 0;
	}
	//std::cout << "end" << std::endl;

	cv::imwrite(nameImage + "_result" + ".jpg", A);

	double metricsOne{ countMetricsOne(nameImage + "_result", nameImage + "-segments") };

	double metricsJaccard{ countMetricsJaccard(nameImage + "_result", nameImage + "-segments") };

	std::cout << metricsOne << std::endl;
	std::cout << metricsJaccard << std::endl;

	//cv::imshow("window", A);
	//cv::waitKey();

	return 0;
}

bool bfs() {
	int qh{ 0 }, qt{ 0 };

	q[qt++] = s;
	memset(d, -1, n * sizeof d[0]);
	d[s] = 0;

	while (qh < qt) {
		int v{ q[qh++] };
		for (int to{ 0 }; to < n; ++to) {
			if (d[to] == -1 && f[v][to] < c[v][to]) {
				q[qt++] = to;
				d[to] = d[v] + 1;
			}
		}
	}
	return d[t] != -1;
}

int dfs(int v, int u) {
	if (!u) {
		return 0;
	}
	if (v == t) {
		return u;
	}

	for (int& to{ ptr[v] }; to < n; ++to) {
		if (d[to] != d[v] + 1) {
			continue;
		}
		int pushed{ dfs(to, fminf(u, c[v][to] - f[v][to])) };

		if (pushed) {
			f[v][to] += pushed;
			f[to][v] -= pushed;

			return pushed;
		}
	}
	return 0;
}

int dinic() {
	int u{ 0 };
	while (true) {
		if (!bfs()) {
			break;
		}

		memset(ptr, 0, n * sizeof ptr[0]);

		while (int pushed = dfs(s, inf)) {
			u += pushed;
		}
	}
	return u;
}

void RemNet()
{
	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j < n; ++j)
			if (i >= j)
				rn[i][j] = 0;
			else rn[i][j] = c[i][j] - f[i][j];
	}
}

void SearchNear(int vert)
{
	mcS[vert] = 1;
	for (int i = 0; i < n; ++i)
		if (rn[vert][i] > 0)
			if (mcS[i] == 0)
				SearchNear(i);
}

void MinCut()
{
	for (int i = 0; i < n; ++i)
		mcS[i] = 0;

	SearchNear(0);

	for (int i = 0; i < n; ++i)
		if (mcS[i] == 0)
			mcT[i] = 1;
		else mcT[i] = 0;
}

int getMaxFlow(const std::string graphAddress) {
	int v{};
	std::fstream file{ graphAddress };
	//file.open("test_1.txt");

	file >> n;
	file >> v;

	s = 0;
	t = n - 1;

	for (int i{ 0 }; i < v; ++i) {
		int a{}, b{}, cost{};
		file >> a;
		file >> b;
		//a -= 1;
		//b -= 1;
		file >> cost;

		c[a][b] = cost;

	}
	file.close();

	int f;

	f = dinic();

	return f;
}

void getImage(const std::string imageAddress, const std::vector<std::array<int, 2>>& background, const std::vector<std::array<int, 2>>& object) {
	cv::Mat img{ cv::imread(imageAddress + ".jpg", cv::IMREAD_GRAYSCALE) };

	rowsImg = img.rows;
	colsImg = img.cols;

	//cv::imshow("window", img);
	//cv::waitKey();

	std::vector<std::array<int, 3>> graph{};
	int vertexNum{ 0 };
	int edgeNum{ 0 };

	std::cout << img.rows << ' ' << img.cols << std::endl;

	std::array<double, 255> probabilityBackground{};

	for (int i{ 0 }; i < probabilityBackground.size(); ++i) {
		probabilityBackground.at(i) = 0.1e-6;
	}

	for (int i{ 0 }; i < background.size(); ++i) {
		int intensity{ static_cast<int>(img.at<uchar>(background.at(i).at(0), background.at(i).at(1))) };

		++probabilityBackground.at(intensity);
	}

	for (int i{ 0 }; i < img.rows; ++i) {
		for (int j{ 0 }; j < img.cols; ++j) {
			int sourceVertex{ vertexNum };
			int currentVertex{ i * img.cols + j + 1 };
			int wieghEdge{};

			if (checkAffiliation(background, i, j)) {
				wieghEdge = 0;
				graph.push_back(std::array<int, 3>{sourceVertex, currentVertex, wieghEdge });

				++edgeNum;
			}
			else if (!checkAffiliation(object, i, j)) {
				int intensity{ static_cast<int>(img.at<uchar>(i, j)) };
				wieghEdge = static_cast<int>(-lambda * log(probabilityBackground.at(intensity) / background.size()) * 100);
				//if (wieghEdge >= 0) {
				graph.push_back(std::array<int, 3>{sourceVertex, currentVertex, wieghEdge });

				++edgeNum;
				//}
			}
		}
	}
	++vertexNum;

	for (int i{ 0 }; i < img.rows; ++i) {
		for (int j{ 0 }; j < img.cols; ++j) {
			if (j < img.cols - 1) {
				int currentVertex{ i * img.cols + j + 1 };
				int rightVertex{ i * img.cols + j + 2 };
				int wieghEdge{ static_cast<int>(exp(-(pow(abs(static_cast<int>(img.at<uchar>(i, j)) - static_cast<int>(img.at<uchar>(i, j + 1))), 2)) / (2 * pow(delta, 2))) * 100) };

				graph.push_back(std::array<int, 3>{currentVertex, rightVertex, wieghEdge });
				++edgeNum;
			}

			if (i < img.rows - 1) {
				int currentVertex{ i * img.cols + j + 1 };
				int downVertex{ ((i + 1) * img.cols + j + 1) };
				int wieghEdge{ static_cast<int>(exp(-(pow(abs(static_cast<int>(img.at<uchar>(i, j)) - static_cast<int>(img.at<uchar>(i + 1, j))), 2)) / (2 * pow(delta, 2))) * 100) };

				graph.push_back(std::array<int, 3>{currentVertex, downVertex, wieghEdge });
				++edgeNum;
			}

			if (j > 0) {
				int currentVertex{ i * img.cols + j + 1 };
				int leftVertex{ (i * img.cols + j) };
				int wieghEdge{ static_cast<int>(exp(-(pow(abs(static_cast<int>(img.at<uchar>(i, j)) - static_cast<int>(img.at<uchar>(i, j - 1))), 2)) / (2 * pow(delta, 2))) * 100) };

				graph.push_back(std::array<int, 3>{currentVertex, leftVertex, wieghEdge });
				++edgeNum;
			}

			if (i > 0) {
				int currentVertex{ i * img.cols + j + 1 };
				int upVertex{ ((i - 1) * img.cols + j + 1) };
				int wieghEdge{ static_cast<int>(exp(-(pow(abs(static_cast<int>(img.at<uchar>(i, j)) - static_cast<int>(img.at<uchar>(i - 1, j))), 2)) / (2 * pow(delta, 2))) * 100) };

				graph.push_back(std::array<int, 3>{currentVertex, upVertex, wieghEdge });
				++edgeNum;
			}

			if (eightEdge) {

				if ((i > 0) && (j < img.cols - 1)) {
					int currentVertex{ i * img.cols + j + 1 };
					int upRightVertex{ ((i - 1) * img.cols + j + 2) };
					int wieghEdge{ static_cast<int>(exp(-(pow(abs(static_cast<int>(img.at<uchar>(i, j)) - static_cast<int>(img.at<uchar>(i - 1, j + 1))), 2)) / (2 * pow(delta, 2)))* (1 / sqrt(2)) * 100) };

					graph.push_back(std::array<int, 3>{currentVertex, upRightVertex, wieghEdge });
					++edgeNum;
				}

				if ((i < img.rows - 1) && (j < img.cols - 1)) {
					int currentVertex{ i * img.cols + j + 1 };
					int downRightVertex{ ((i + 1) * img.cols + j + 2) };
					int wieghEdge{ static_cast<int>(exp(-(pow(abs(static_cast<int>(img.at<uchar>(i, j)) - static_cast<int>(img.at<uchar>(i + 1, j + 1))), 2)) / (2 * pow(delta, 2)))* (1 / sqrt(2)) * 100) };

					graph.push_back(std::array<int, 3>{currentVertex, downRightVertex, wieghEdge });
					++edgeNum;
				}

				if ((i < img.rows - 1) && (j > 0)) {
					int currentVertex{ i * img.cols + j + 1 };
					int downLeftVertex{ ((i + 1) * img.cols + j) };
					int wieghEdge{ static_cast<int>(exp(-(pow(abs(static_cast<int>(img.at<uchar>(i, j)) - static_cast<int>(img.at<uchar>(i + 1, j - 1))), 2)) / (2 * pow(delta, 2)))* (1 / sqrt(2)) * 100) };

					graph.push_back(std::array<int, 3>{currentVertex, downLeftVertex, wieghEdge });
					++edgeNum;
				}

				if ((i > 0) && (j > 0)) {
					int currentVertex{ i * img.cols + j + 1 };
					int downLeftVertex{ ((i - 1) * img.cols + j) };
					int wieghEdge{ static_cast<int>(exp(-(pow(abs(static_cast<int>(img.at<uchar>(i, j)) - static_cast<int>(img.at<uchar>(i - 1, j - 1))), 2)) / (2 * pow(delta, 2)))* (1 / sqrt(2)) * 100) };

					graph.push_back(std::array<int, 3>{currentVertex, downLeftVertex, wieghEdge });
					++edgeNum;
				}
			}
			++vertexNum;
		}
	}

	std::array<double, 255> probabilityObject{};


	for (int i{ 0 }; i < probabilityObject.size(); ++i) {
		probabilityObject.at(i) = 0.1e-6;
	}

	for (int i{ 0 }; i < object.size(); ++i) {
		int intensity{ static_cast<int>(img.at<uchar>(object.at(i).at(0), object.at(i).at(1))) };

		++probabilityObject.at(intensity);
	}

	for (int i{ 0 }; i < img.rows; ++i) {
		for (int j{ 0 }; j < img.cols; ++j) {
			int currentVertex{ i * img.cols + j + 1 };
			int sinkVertex{ vertexNum };
			int wieghEdge{};

			if (checkAffiliation(object, i, j)) {
				wieghEdge = 0;
				graph.push_back(std::array<int, 3>{currentVertex, sinkVertex, wieghEdge });

				++edgeNum;
			}
			else if (!checkAffiliation(background, i, j)) {
				int intensity{ static_cast<int>(img.at<uchar>(i, j)) };
				wieghEdge = static_cast<int>(-lambda * log(probabilityObject.at(intensity) / object.size()) * 100); // back - obj
				//if (wieghEdge >= 0) {
				graph.push_back(std::array<int, 3>{currentVertex, sinkVertex, wieghEdge });

				++edgeNum;
				//}
			}
		}
	}
	++vertexNum;

	std::ofstream outf(imageAddress + ".txt");
	outf << vertexNum << ' ' << edgeNum << std::endl;
	for (int i{ 0 }; i < graph.size(); ++i) {
		outf << graph.at(i).at(0) << ' ' << graph.at(i).at(1) << ' ' << graph.at(i).at(2) << std::endl;
	}

	outf.close();
}

void setBackgroundAndObject(std::vector<std::array<int, 2>>& background, std::vector<std::array<int, 2>>& object) {
	//bannana
	/*for (int i{ 0 }; i < 28; ++i) {
		for (int j{ 0 }; j < 74; ++j) {
			background.push_back(std::array<int, 2> {i, j});
		}
	}
	for (int i{ 68 }; i < 74; ++i) {
		for (int j{ 0 }; j < 99; ++j) {
			background.push_back(std::array<int, 2> {i, j});
		}
	}

	for (int i{ 44 }; i < 61; ++i) {
		for (int j{ 23 }; j < 68; ++j) {
			object.push_back(std::array<int, 2> {i, j});
		}
	}
	for (int i{ 38 }; i < 56; ++i) {
		for (int j{ 70 }; j < 78; ++j) {
			object.push_back(std::array<int, 2> {i, j});
		}
	}
	for (int i{ 23 }; i < 48; ++i) {
		for (int j{ 80 }; j < 88; ++j) {
			object.push_back(std::array<int, 2> {i, j});
		}
	}*/


	////banana
	///*for (int i{ 0 }; i < 28; ++i) {
	//	for (int j{ 0 }; j < 10; ++j) {
	//		background.push_back(std::array<int, 2> {i, j});
	//	}
	//}
	//for (int i{ 68 }; i < 71; ++i) {
	//	for (int j{ 10 }; j < 40; ++j) {
	//		background.push_back(std::array<int, 2> {i, j});
	//	}
	//}

	//for (int i{ 55 }; i < 61; ++i) {
	//	for (int j{ 23 }; j < 68; ++j) {
	//		object.push_back(std::array<int, 2> {i, j});
	//	}
	//}
	//for (int i{ 38 }; i < 56; ++i) {
	//	for (int j{ 72 }; j < 78; ++j) {
	//		object.push_back(std::array<int, 2> {i, j});
	//	}
	//}
	//for (int i{ 23 }; i < 48; ++i) {
	//	for (int j{ 80 }; j < 88; ++j) {
	//		object.push_back(std::array<int, 2> {i, j});
	//	}
	//}*/

	////stone
	//for (int i{ 0 }; i <= 15; i += 3) {
	//	for (int j{ 0 }; j <= 15; j += 3) {
	//		background.push_back(std::array<int, 2> {i, j});
	//	}
	//}
	//for (int i{ 68 }; i <= 74; i += 3) {
	//	for (int j{ 52 }; j <= 61; j += 3) {
	//		background.push_back(std::array<int, 2> {i, j});
	//	}
	//}

	//for (int i{ 27 }; i <= 57; i += 5) {
	//	for (int j{ 39 }; j <= 63; j += 5) {
	//		object.push_back(std::array<int, 2> {i, j});
	//	}
	//}

	//for (int j{ 55 }; j <= 65; ++j) {
	//	object.push_back(std::array<int, 2> {56, j});
	//	object.push_back(std::array<int, 2> {58, j});
	//}

	////music
	//for (int i{ 0 }; i <= 74; i += 10) {
	//	for (int j{ 0 }; j <= 22; j += 3) {
	//		background.push_back(std::array<int, 2> {i, j});
	//	}
	//}

	//for (int i{ 12 }; i <= 60; i += 5) {
	//	for (int j{ 32 }; j <= 61; j += 3) {
	//		object.push_back(std::array<int, 2> {i, j});
	//	}
	//}

	//elefant
	for (int i{ 0 }; i <= 73; i += 6) {
		for (int j{ 0 }; j <= 12; j += 1) {
			background.push_back(std::array<int, 2> {i, j});
		}
	}

	for (int i{ 25 }; i <= 43; i += 3) {
		for (int j{ 38 }; j <= 65; j += 3) {
			object.push_back(std::array<int, 2> {i, j});
		}
	}


	//for (int i{ 20 }; i < 60; ++i) {
	//	background.push_back(std::array<int, 2> {20, i});
	//	//background.push_back(std::array<int, 2> {21, i});
	//	//background.push_back(std::array<int, 2> {22, i});
	//}
	//for (int i{ 0 }; i < 30; ++i) {
	//	background.push_back(std::array<int, 2> {4, i});
	//	//background.push_back(std::array<int, 2> {5, i});
	//	//background.push_back(std::array<int, 2> {6, i});
	//}

	//for (int i{ 8 }; i < 70; ++i) {
	//	object.push_back(std::array<int, 2> {45, i});
	//	object.push_back(std::array<int, 2> {46, i});
	//	object.push_back(std::array<int, 2> {47, i});
	//	object.push_back(std::array<int, 2> {48, i});
	//}

	std::cout << "Would you like to add more pixels to bkg/obj? - y/n" << std::endl;
	char add;
	while (true)
	{
		std::cin >> add;
		if (add == 'n')
			break;
		if (add == 'y')
		{

			while (true)
			{
				std::cout << "Add to bkg/add to obj/exit: b/o/e" << std::endl;
				std::cin >> add;
				if (add == 'b')
				{
					int x, y;
					std::cout << "Insert (x,y): ";
					std::cin >> x >> y;
					background.push_back(std::array<int, 2> {x, y});
				}
				if (add == 'o')
				{
					int x, y;
					std::cout << "Insert (x,y): ";
					std::cin >> x >> y;
					object.push_back(std::array<int, 2> {x, y});
				}
				if (add == 'e')
					break;
			}
			break;
		}
		else std::cout << "Insert correct value: ";
	}


	/*
	for (int i{ 0 }; i < 70; ++i) {
		background.push_back(std::array<int, 2> {20, i});
	}

	for (int i{ 45 }; i < 61; i += 5) {
		for (int j{ 25 }; j < 50; ++j) {
			object.push_back(std::array<int, 2> {i, j});
		}
	}*/


	//moon
	/*for (int i{ 0 }; i < 25; ++i) {
		for (int j{ 0 }; j < 95; ++j) {
			background.push_back(std::array<int, 2> {i, j});
		}
	}

	for (int i{ 33 }; i < 45; ++i) {
		for (int j{ 37 }; j < 57; ++j) {
			object.push_back(std::array<int, 2> {i, j});
		}
	}*/
}

bool checkAffiliation(const std::vector<std::array<int, 2>>& multiplicity, int i, int j) {
	for (int k{ 0 }; k < multiplicity.size(); ++k) {
		if ((multiplicity.at(k).at(0) == i) && (multiplicity.at(k).at(1) == j)) {
			return true;
		}
	}
	return false;
}

std::array<int, 2> decoder(int num, int cols) {
	std::array<int, 2> cord{};

	--num;
	int i{ 0 };
	while (num >= cols * i) {
		++i;
	}
	--i;

	num -= cols * i;

	int j{ num };

	return std::array<int, 2>{ i, j };
}

double countMetricsOne(const std::string nameImageReuslt, const std::string nameImageSegment) {
	cv::Mat imageResult{ cv::imread(nameImageReuslt + ".jpg", cv::IMREAD_GRAYSCALE) };
	cv::Mat imageSegment{ cv::imread(nameImageSegment + ".jpg", cv::IMREAD_GRAYSCALE) };

	double samePixelNum{};
	const int pixelNum{ imageResult.rows * imageResult.cols };

	for (int i{ 0 }; i < imageResult.rows; ++i) {
		for (int j{ 0 }; j < imageResult.cols; ++j) {
			if (imageResult.at<uchar>(i, j) == imageSegment.at<uchar>(i, j)) {
				++samePixelNum;
			}
		}
	}

	return samePixelNum / pixelNum;
}

double countMetricsJaccard(const std::string nameImageReuslt, const std::string nameImageSegment) {
	cv::Mat imageResult{ cv::imread(nameImageReuslt + ".jpg", cv::IMREAD_GRAYSCALE) };
	cv::Mat imageSegment{ cv::imread(nameImageSegment + ".jpg", cv::IMREAD_GRAYSCALE) };

	double intersectionPixelNum{};
	double unionPixelNum{};

	for (int i{ 0 }; i < imageResult.rows; ++i) {
		for (int j{ 0 }; j < imageResult.cols; ++j) {
			if ((imageResult.at<uchar>(i, j) > 50) && (imageSegment.at<uchar>(i, j) > 50)) {
				++intersectionPixelNum;
				++unionPixelNum;
			}
			else if ((imageResult.at<uchar>(i, j) > 50) || (imageSegment.at<uchar>(i, j) > 50)) {
				++unionPixelNum;
			}
		}
	}

	return intersectionPixelNum / unionPixelNum;
}
