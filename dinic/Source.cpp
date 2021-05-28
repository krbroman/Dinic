#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <opencv2/opencv.hpp>

static const int maxn{ 10000 };
static const int inf{ 1000000000 };

static int n{}, c[maxn][maxn]{}, f[maxn][maxn]{}, s{}, t{}, d[maxn]{}, ptr[maxn]{}, q[maxn]{}, rn[maxn][maxn]{}, mcS[maxn]{}, mcT[maxn]{};

static int rowsImg{}, colsImg{};

static const int delta{ 5 };
static const int lambda{ 20 };

bool bfs();
int dfs(int v, int u);
int dinic();
void RemNet();
void SearchNear(int vert);
void MinCut();

std::array<int, 2> decoder(int num, int cols);

int getMaxFlow(const std::string graphAddress);

std::string getImage(std::string imageAddress, const std::vector<std::array<int, 2>>& background, const std::vector<std::array<int, 2>>& object);

void setBackgroundAndObject(std::vector<std::array<int, 2>>& background, std::vector<std::array<int, 2>>& object);

bool checkAffiliation(const std::vector<std::array<int, 2>>& multiplicity, int i, int j);

int main() {
	std::vector<std::array<int, 2>> background{};
	std::vector<std::array<int, 2>> object{};

	setBackgroundAndObject(background, object);

	std::string graphAddress{ getImage("banana1-gr-100.jpg", background, object) };
	//std::string graphAddress{};
	std::cout << "end" << std::endl;
	//std::cout << graphAddress << std::endl;

	//graphAddress = "banana1-gr-100.txt";
	//graphAddress = "test_1.txt";

	int maxFlow{ getMaxFlow(graphAddress) };

	std::cout << maxFlow << std::endl;

	RemNet();
	MinCut();
	

	cv::Mat A{ cv::imread("banana1-gr-100.jpg", cv::IMREAD_GRAYSCALE) };
	for (int i = 1; i < n-1; ++i)
	{
		std::array<int, 2> xy = decoder(i, colsImg);


		if(mcS[i] == 1)
			A.at<uchar>(xy[0], xy[1]) = 250;
		else A.at<uchar>(xy[0], xy[1]) = 0;
		//A.at<uchar>(xy[0], xy[1]) = 250;
	}
	
	cv::imshow("window", A);
	cv::waitKey(); 

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
			if(mcS[i] == 0)
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

std::string getImage(std::string imageAddress, const std::vector<std::array<int, 2>>& background, const std::vector<std::array<int, 2>>& object) {
	cv::Mat img{ cv::imread(imageAddress, cv::IMREAD_GRAYSCALE) };

	/*cv::imshow("window", img);
	cv::waitKey();*/

	rowsImg = img.rows;
	colsImg = img.cols;

	std::vector<std::array<int, 3>> graph{};
	int vertexNum{ 0 };
	int edgeNum{ 0 };

	std::cout << img.rows << ' ' << img.cols << std::endl;

	std::array<double, 255> probabilityBackground{};

	for (int i{ 0 }; i < probabilityBackground.size(); ++i) {
		probabilityBackground.at(i) = 0.1;
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
			++vertexNum;
		}
	}

	std::array<double, 255> probabilityObject{};


	for (int i{ 0 }; i < probabilityObject.size(); ++i) {
		probabilityObject.at(i) = 0.1;
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
				wieghEdge = static_cast<int>(-lambda * log(probabilityObject.at(intensity) / background.size()) * 100);
				//if (wieghEdge >= 0) {
				graph.push_back(std::array<int, 3>{currentVertex, sinkVertex, wieghEdge });

				++edgeNum;
				//}
			}
		}
	}
	++vertexNum;

	const std::string graphAddress{ imageAddress.erase(imageAddress.size() - 4) + ".txt" };

	std::ofstream outf(graphAddress);
	outf << vertexNum << ' ' << edgeNum << std::endl;
	for (int i{ 0 }; i < graph.size(); ++i) {
		outf << graph.at(i).at(0) << ' ' << graph.at(i).at(1) << ' ' << graph.at(i).at(2) << std::endl;
	}

	outf.close();

	return graphAddress;

	//cv::imshow("window", img);
	//cv::waitKey();
}

void setBackgroundAndObject(std::vector<std::array<int, 2>>& background, std::vector<std::array<int, 2>>& object) {
	background.push_back(std::array<int, 2> {2, 6});
	background.push_back(std::array<int, 2> {15, 29});
	background.push_back(std::array<int, 2> {29, 37});
	background.push_back(std::array<int, 2> {4, 55});
	background.push_back(std::array<int, 2> {65, 94});

	object.push_back(std::array<int, 2> {43, 13});
	object.push_back(std::array<int, 2> {54, 36});
	object.push_back(std::array<int, 2> {55, 59});
	object.push_back(std::array<int, 2> {27, 88});
	object.push_back(std::array<int, 2> {43, 82});
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