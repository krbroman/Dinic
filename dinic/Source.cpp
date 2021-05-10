using namespace std;
#include <iostream>
#include <fstream>
#include <cmath>
#include <stdio.h>      
#include <time.h>       
#include <math.h> 
const int maxn = 100; 
const int inf = 1000000000;

int n, c[maxn][maxn], f[maxn][maxn], s, t, d[maxn], ptr[maxn], q[maxn];

bool bfs() 
{
	int qh = 0, qt = 0;

	q[qt++] = s;
	memset(d, -1, n * sizeof d[0]);
	d[s] = 0;

	while (qh < qt)
	{
		int v = q[qh++];
		for (int to = 0; to < n; ++to)
			if (d[to] == -1 && f[v][to] < c[v][to]) 
			{
				q[qt++] = to;
				d[to] = d[v] + 1;
			}
	}
	return d[t] != -1;
}

int dfs(int v, int u)
{
	if (!u)  
		return 0;	
	if (v == t)  
		return u;

	for (int& to = ptr[v]; to < n; ++to) 
	{
		if (d[to] != d[v] + 1)  continue;
		int pushed = dfs(to, fminf(u, c[v][to] - f[v][to]));

		if (pushed) 
		{
			f[v][to] += pushed;
			f[to][v] -= pushed;

			return pushed;
		}
	}
	return 0;
}

int dinic() 
{
	int u = 0;
	while (true)
	{
		if (!bfs())  
			break;

		memset(ptr, 0, n * sizeof ptr[0]);

		while (int pushed = dfs(s, inf))
			u += pushed;
	}
	return u;
}

void main()
{
	int v;
	fstream file;
	file.open("test.txt");
	if (!file.is_open())
	{
		return;
	}

	file >> n;
	file >> v;
	
	s = 0;
	t = n-1;

	for (int i = 0; i < v; ++i)
	{
		int a, b, cost;
		file >> a;
		file >> b;
		a -= 1;
		b -= 1;
		file >> cost;

		c[a][b] = cost;
		
	}
	
	clock_t t;
	int f;
	t = clock();
	f = dinic();
	t = clock() - t;
	printf("%f seconds\n", t, ((float)t) / CLOCKS_PER_SEC);
	cout << "Answer: " << f << endl;
	file.close();
}