#define _CRT_SECURE_NO_WARNINGS // 禁用安全警告
#pragma warning(disable : 6031)
#pragma warning(disable : 26451)
#pragma warning(disable : 26495)
#pragma warning(disable : 26812) 
#pragma comment(lib, "user32.lib") // user32.lib是Windows系统的一个核心库，
#include <stdio.h>                 // 包含了许多与窗口消息、鼠标键盘事件处理等相关的函数
#include <stdlib.h>
#include <conio.h>
#include <graphics.h>
#include <cmath>
#include <string.h>
#include <vector>
#include <algorithm>
#include <utility>
#include <string>
#include <time.h>
#include <sstream>
#include <map>
using namespace std;
#define Width 1440 // 宏定义，窗口的宽度为1440像素，高度为900像素
#define Height 900 // 在easyx中通过initgraph(Width, Height)来创建指定大小的窗口
#define PI 3.1415926535898 // 计算角度转换、三角函数等场景中会用到
#define GetKey(X) (GetAsyncKeyState(X) & 0x8000)
const int Watcher_x = 1000, Back_wall_x = -100; // 定义摄像头的位置和投影面的三维的x坐标1000和-100

struct Point
{
    double x, y, z;
    POINT trans(const Point *offset) const // trans函数将三维点投影到二维屏幕上，offset表示偏移量

    // 将三维的点投影到二维的屏幕上
    { // 变换//投影//透视
        double X, Y, Z;
        X = x + offset->x, Y = y + offset->y, Z = z + offset->z;
        Y = Y * (Watcher_x - Back_wall_x) / (Watcher_x - X);
        Z = Z * (Watcher_x - Back_wall_x) / (Watcher_x - X);
        return {int(Y), int(-Z)};
    }
    Point operator*=(double n) // 等比缩放n倍
    {
        x *= n, y *= n, z *= n;
        return *this; // this表示指向当前对象的指针，直接返回变换后的x，y，z
    }
    Point operator-() // 反向
    {
        x = -x, y = -y, z = -z;
        return {x, y, z};
    }
    friend Point operator+(const Point &a, const Point &b);
    friend Point operator-(const Point &a, const Point &b);
    friend Point operator*(const Point &a, double n);
    friend Point operator/(const Point &a, double n);
};
Point operator+(const Point &a, const Point &b) // 计算两个点的距离之和
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}
Point operator-(const Point &a, const Point &b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}
Point operator*(const Point &a, double n)
{
    return {a.x * n, a.y * n, a.z * n};
}
Point operator/(const Point &a, double n)
{
    return {a.x / n, a.y / n, a.z / n};
}
Point cross(const Point &u, const Point &v) // 计算两个三维向量的叉乘结果
{
    return {u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x};
}
double Dist(const Point &a, const Point &b) // 计算dist，两点间的距离
{
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z));
}
double Cos(const Point &a, const Point &b) // 计算两个向量夹角的余弦值
{
    Point origin = {0, 0, 0};
    double alpha = (a.x * b.x + a.y * b.y + a.z * b.z) / (Dist(a, origin) * Dist(b, origin)); // 先算出两个向量的点积，然后
    return (alpha > 0.99) ? 0.99 : (alpha < -0.99) ? -0.99                                    // 除以两个向量的模长，就求出了余弦值
                                                   : alpha;   // 余弦值的范围是[-1,1]，防止超出范围，三元运算符的套用
                                                              // condition ? true_value : false_value形式
}
double DistToWatcher(const Point &p)
{
    Point watcher = {Watcher_x, 0, 0};
    return Dist(watcher, p);
} 
struct Plane // 表示三维空间的一个平面
{
    Plane() = default; // 初始化
    Plane(const Point *a, const Point *b, const Point *c, const Point *d)
    {
        bindPoint(a, b, c, d); // Plane函数接受并储存四个顶点的指针于p[]中
    }
    void bindPoint(const Point *a, const Point *b, const Point *c, const Point *d)
    {
        p[0] = a, p[1] = b, p[2] = c, p[3] = d; 
    }
    void bindOffset(const Point *off)
    {
        offset = off;
    }
    void setColor(COLORREF col)
    {
        color = col;
    }
    Point Center(void) const
    { // p[0] p[2]为对角线
        Point center = (*p[0] + *p[2]) / 2;
        return center + *offset;
    }
    void draw(void) const
    {
        setfillcolor(color); // easyx标准库的函数，用于填充cube的颜色
        setlinecolor(RGB(100, 100, 100)); // 边缘线条为灰色
        POINT point[4] = {p[0]->trans(offset), p[1]->trans(offset), p[2]->trans(offset), p[3]->trans(offset)}; // p[]来访问变换好的投影
        fillpolygon(point, 4); // easyx的函数，绘制四个已经定义并透视好的四个顶点，填充绘制多边形
    }
    friend struct RubikCube;
    friend bool operator>(const Plane &a, const Plane &b); // 距离视点远近; >: far
private: // 成员变量，放在最后有可读性等等
    const Point *p[4];
    const Point *offset; // 定义的一个偏移量，作用是将局部坐标转化为全局坐标
    COLORREF color = RGB(50, 50, 50); // RGB(20, 20, 20)
};

bool operator>(const Plane &a, const Plane &b)
{
    Point C_a = a.Center(), C_b = b.Center(); // 因为struct结构体中，默认是public的，而class是 类 默认是private
    return DistToWatcher(C_a) > DistToWatcher(C_b);  // 在struct中，只有摆明了是private才是结构体内部私有的
}
struct Cube
{
    Cube() // vertex表示魔方的八个顶点，根据空间关系绑在一起
    {
        plane[0].bindPoint(&vertex[0], &vertex[1], &vertex[2], &vertex[3]); // Front
        plane[1].bindPoint(&vertex[1], &vertex[5], &vertex[6], &vertex[2]); // Right
        plane[2].bindPoint(&vertex[3], &vertex[2], &vertex[6], &vertex[7]); // Down
        plane[3].bindPoint(&vertex[0], &vertex[1], &vertex[5], &vertex[4]); // Up
        plane[4].bindPoint(&vertex[4], &vertex[5], &vertex[6], &vertex[7]); // Back
        plane[5].bindPoint(&vertex[0], &vertex[3], &vertex[7], &vertex[4]); // Left
    }
    void setOffset(Point Offset)
    {
        offset = Offset;
        for (int i = 0; i < P_N; i ++)
            plane[i].bindOffset(&offset);
    }
    void rotateX(double degree)
    { // 顺时针
        for (int i = 0; i < N; i++)
        {
            double y = vertex[i].y, z = vertex[i].z;
            double d = degree / 180 * PI; // z->y，转化为弧度
            vertex[i].y = y * cos(d) + z * sin(d); // 三角函数sin(),cos(),有对应的绕轴旋转公式
            vertex[i].z = z * cos(d) - y * sin(d);
        }
    }
    void rotateY(double degree)
    { // 顺时针
        for (int i = 0; i < N; i++)
        {
            double x = vertex[i].x, z = vertex[i].z;
            double d = degree / 180 * PI; // x->z
            vertex[i].z = z * cos(d) + x * sin(d);
            vertex[i].x = x * cos(d) - z * sin(d);
        }
    }
    void rotateZ(double degree)
    { // 顺时针
        for (int i = 0; i < N; i++)
        {
            double x = vertex[i].x, y = vertex[i].y;
            double d = degree / 180 * PI; // y->x
            vertex[i].x = x * cos(d) + y * sin(d);
            vertex[i].y = y * cos(d) - x * sin(d);
        }
    }
    void rotateX_Local(double degree, const Point A = {0, 0, 0}, const Point B = {0, 0, 0}, bool bUseAB = false)
    {                                       // 弧度
        Point a = vertex[4], b = vertex[0]; // 预先存值，否则循环中改变
        if (bUseAB)
            a = A, b = B;
        for (int i = 0; i < N; i++)
            vertex[i] = rotateByVector(vertex[i], a, b, degree);
    }
    void rotateY_Local(double degree, const Point A = {0, 0, 0}, const Point B = {0, 0, 0}, bool bUseAB = false)
    {                                       // 弧度
        Point a = vertex[0], b = vertex[1]; // 预先存值，否则循环中改变
        if (bUseAB)
            a = A, b = B;
        for (int i = 0; i < N; i++)
            vertex[i] = rotateByVector(vertex[i], a, b, degree);
    }
    void rotateZ_Local(double degree, const Point A = {0, 0, 0}, const Point B = {0, 0, 0}, bool bUseAB = false)
    {                                       // 弧度
        Point a = vertex[2], b = vertex[1]; // 预先存值，否则循环中改变
        if (bUseAB)
            a = A, b = B;
        for (int i = 0; i < N; i++)
            vertex[i] = rotateByVector(vertex[i], a, b, degree);
    }
    static void rotateX(Point &p, double degree)
    { // 顺时针
        double y = p.y, z = p.z;
        double d = degree / 180 * PI; // z->y
        p.y = y * cos(d) + z * sin(d);
        p.z = z * cos(d) - y * sin(d);
    }
    static void rotateY(Point &p, double degree)
    { // 顺时针
        double x = p.x, z = p.z;
        double d = degree / 180 * PI; // x->z
        p.z = z * cos(d) + x * sin(d);
        p.x = x * cos(d) - z * sin(d);
    }
    static void rotateZ(Point &p, double degree)
    { // 顺时针
        double x = p.x, y = p.y;
        double d = degree / 180 * PI; // y->x
        p.x = x * cos(d) + y * sin(d);
        p.y = y * cos(d) - x * sin(d);
    }
    static Point rotateByVector(const Point &V, const Point &a, const Point &b, double T) // 罗德里格旋转公式，绕任意轴旋转
    {
        Point K, ans;
        K = a - b;
        K = K / Dist(a, b);
        double x = K.x, y = K.y, z = K.z;
        double u = V.x, v = V.y, w = V.z;
        ans.x = u * cos(T) + (y * w - z * v) * sin(T) + x * (x * u + y * v + z * w) * (1 - cos(T));
        ans.y = v * cos(T) + (z * u - x * w) * sin(T) + y * (x * u + y * v + z * w) * (1 - cos(T));
        ans.z = w * cos(T) + (x * v - y * u) * sin(T) + z * (x * u + y * v + z * w) * (1 - cos(T));
        return ans;
    }
    void showCube(void)
    {
        sort(plane, plane + P_N, [](const Plane &a, const Plane &b)
             { return a > b; }); // 画家消隐算法
        for (int i = 0; i < P_N; i++)
            plane[i].draw();
    }
    Plane *getPlanes(void)
    {
        return plane;
    }
    friend struct RubikCube;

private:
    const double e = 50;
    static const int N = 8;
    static const int P_N = 6;
    Point offset = {0, 0, 0}; //
    Point vertex[N] = {{e, -e, e}, {e, e, e}, {e, e, -e}, {e, -e, -e}, {-e, -e, e}, {-e, e, e}, {-e, e, -e}, {-e, -e, -e}};
    Plane plane[P_N];
};
enum Dir : int // 枚举类型
{
    Left, // 会自动从0开始往后赋值
    Front,
    Right,
    Back,
    Up,
    Down,
    Still
};
struct RubikCube
{
    RubikCube()
    {
        for (int k = 0; k < D; k++)
            for (int i = 0; i < D; i++)
                for (int j = 0; j < D; j++)
                {
                    offset[k][i][j] = {1.0 - k, j - 1.0, 1.0 - i};
                    offset[k][i][j] *= E;
                    offset[k][i][j] = offset[k][i][j];
                    cube[k][i][j].setOffset(offset[k][i][j]);
                }
        // 上色
        for (int i = 0; i < D; i++)
            for (int j = 0; j < D; j++)
                cube[0][i][j].plane[0].setColor(RGB(235, 235, 235)); // White//Front
        for (int i = 0; i < D; i++)
            for (int j = 0; j < D; j++)
                cube[2][i][j].plane[4].setColor(RGB(255, 255, 0)); // Yellow//Back
        for (int i = 0; i < D; i++)
            for (int k = 0; k < D; k++)
                cube[k][i][0].plane[5].setColor(RGB(235, 0, 0)); // Red//Left
        for (int i = 0; i < D; i++)
            for (int k = 0; k < D; k++)
                cube[k][i][2].plane[1].setColor(RGB(255, 128, 50)); // Orange//Right
        for (int j = 0; j < D; j++)
            for (int k = 0; k < D; k++)
                cube[k][0][j].plane[3].setColor(RGB(0, 255, 0)); // Green//Up
        for (int j = 0; j < D; j++)
            for (int k = 0; k < D; k++)
                cube[k][2][j].plane[2].setColor(RGB(62, 157, 251)); // Blue//Down
    }
    void draw(void) // 画家算法，先绘制较远的画面
    {
        vector<pair<Plane *, double>> v; // 定义的一个向量
        for (int k = 0; k < D; k++) // D = 3
            for (int i = 0; i < D; i++)
                for (int j = 0; j < D; j++)
                {
                    Plane *plane = cube[k][i][j].getPlanes();
                    for (int n = 0; n < 6; n++)
                        v.push_back({&plane[n], DistToWatcher(plane[n].Center())});
                }
        sort(v.begin(), v.end(), [](const pair<Plane *, double> &a, const pair<Plane *, double> &b)
            { return a.second > b.second; });
        int size = v.size();
        for (int i = 0; i < size; i++)
            v[i].first->draw();
    }
    void rotateX(double degree) // 将所有的小块整体绕x轴转动，相当于视角的变换
    {
        for (int k = 0; k < D; k++)
            for (int i = 0; i < D; i++)
                for (int j = 0; j < D; j++)
                {
                    Cube::rotateX(offset[k][i][j], degree);
                    cube[k][i][j].setOffset(offset[k][i][j]);
                    cube[k][i][j].rotateX(degree);
                }
    }
    void rotateY(double degree)
    {
        for (int k = 0; k < D; k++)
            for (int i = 0; i < D; i++)
                for (int j = 0; j < D; j++)
                {
                    Cube::rotateY(offset[k][i][j], degree);
                    cube[k][i][j].setOffset(offset[k][i][j]);
                    cube[k][i][j].rotateY(degree);
                }
    }
    void rotateZ(double degree)
    {
        for (int k = 0; k < D; k++)
            for (int i = 0; i < D; i++)
                for (int j = 0; j < D; j++)
                {
                    Cube::rotateZ(offset[k][i][j], degree);
                    cube[k][i][j].setOffset(offset[k][i][j]);
                    cube[k][i][j].rotateZ(degree);
                }
    }
    void rotateX_Local(double degree, string dir = "front") // 绕特定层的旋转
    {
        const Point &A = offset[1][1][1], &B = *Map[1]; // offset[0][1][1]
        Point law = B - A; // 两个特定的点A,B确定
        if (dir == "back") // 顺转or反转
            law = -law, degree = -degree;
        for (int k = 0; k < D; k++)
            for (int i = 0; i < D; i++)
                for (int j = 0; j < D; j++)
                {
                    Point temp = offset[k][i][j] - offset[1][1][1]; // 自定义的-，用于计算得到一个POINT类型的新的向量，offset[1][1][1]是中心块的坐标
                    if (acos(Cos(temp, law)) / PI * 180 >= 85)
                        continue; // 与法线夹角大于85度
                    offset[k][i][j] = Cube::rotateByVector(offset[k][i][j], A, B, degree);
                    cube[k][i][j].setOffset(offset[k][i][j]); // 更新偏移量
                    cube[k][i][j].rotateX_Local(degree, A, B, true);
                }
    }
    void rotateY_Local(double degree, string dir = "right")
    {
        const Point &A = offset[1][1][1], &B = *Map[2]; // offset[1][1][2]
        Point law = B - A;
        if (dir == "left")
            law = -law, degree = -degree;
        for (int k = 0; k < D; k++)
            for (int i = 0; i < D; i++)
                for (int j = 0; j < D; j++)
                {
                    Point temp = offset[k][i][j] - offset[1][1][1];
                    if (acos(Cos(temp, law)) / PI * 180 >= 85)
                        continue; // 与法线夹角大于85度
                    offset[k][i][j] = Cube::rotateByVector(offset[k][i][j], A, B, degree);
                    cube[k][i][j].setOffset(offset[k][i][j]);
                    cube[k][i][j].rotateY_Local(degree, A, B, true);
                }
    }
    void rotateZ_Local(double degree, string dir = "up")
    {
        const Point &A = offset[1][1][1], &B = *Map[4]; // offset[1][0][1]
        Point law = B - A;
        if (dir == "down")
            law = -law, degree = -degree;
        for (int k = 0; k < D; k++)
            for (int i = 0; i < D; i++)
                for (int j = 0; j < D; j++)
                {
                    Point temp = offset[k][i][j] - offset[1][1][1];
                    if (acos(Cos(temp, law)) / PI * 180 >= 85)
                        continue; // 与法线夹角大于85度
                    offset[k][i][j] = Cube::rotateByVector(offset[k][i][j], A, B, degree);
                    cube[k][i][j].setOffset(offset[k][i][j]);
                    cube[k][i][j].rotateZ_Local(degree, A, B, true);
                }
    }
    void rotate_Local(Dir dir, bool isRev = false)
    {
        if (RotateDir == Still)
            RotateDir = dir, isReverse = isRev; // 方向和是否反转
    }
    void updateRotate(void) // 这个函数通常在动画更新或每一帧渲染时被调用，以实现平滑的旋转效果
    {
        if (RotateDir == Still)
            return;
        void (RubikCube::*func)(double, string) = nullptr;
        string dir;
        switch (RotateDir)
        {
        case Front:
            func = &RubikCube::rotateX_Local;
            dir = "front";
            break;
        case Back:
            func = &RubikCube::rotateX_Local;
            dir = "back";
            break;
        case Right:
            func = &RubikCube::rotateY_Local;
            dir = "right";
            break;
        case Left:
            func = &RubikCube::rotateY_Local;
            dir = "left";
            break;
        case Up:
            func = &RubikCube::rotateZ_Local;
            dir = "up";
            break;
        case Down:
            func = &RubikCube::rotateZ_Local;
            dir = "down";
            break;
        }
        if (func == nullptr)
            return;
        static double degree = 0; //
        // const double step = 0.08;
        int rev = isReverse ? -1 : 1;
        if (degree < PI / 2)
        {
            degree += step;
            (this->*func)(rev * step, dir);
        }
        else
            (this->*func)(rev * (PI / 2 - degree), dir), degree = 0, RotateDir = Still;
    }
    bool isRotateOver(void)
    {
        return RotateDir == Still;
    }
    void setRotateStep(double Step = 0.08)
    {
        step = Step;
    }
    void bindCommand(const string &cmd)
    {
        iss.clear();
        iss.str(cmd);
    }
    void execute(void)
    { // One Step
        if (!iss.good() || !isRotateOver())
            return;
        string cmd;
        iss >> cmd;
        if (cmd == "→") // 改变魔方的轴映射，通常是将魔方顺时针旋转 90 度,这里通过交换 Map 数组中的指针，改变魔方的轴映射
        { // 直接改变3D魔方轴映射
            Point *t = Map[5];
            Map[5] = Map[2];
            Map[2] = Map[4];
            Map[4] = Map[0];
            Map[0] = t;
        }
        else if (cmd == "↓")
        {
            Point *t = Map[3];
            Map[3] = Map[5];
            Map[5] = Map[1];
            Map[1] = Map[4];
            Map[4] = t;
        }
        else
        {
            if (m.find(cmd[0]) != m.end())
                rotate_Local(m[cmd[0]], cmd[1] == '\'' ? true : false);
        }
    }
    bool isExecuteOver(void)
    {
        return !iss.good() && isRotateOver();
    } 

private:
    static const int D = 3;
    static const int E = 100;
    Cube cube[D][D][D];
    Point offset[D][D][D];
    Dir RotateDir = Still;
    bool isReverse = false;
    istringstream iss;
    vector<Point *> Map = {&offset[1][1][0], &offset[0][1][1], &offset[1][1][2], &offset[2][1][1], &offset[1][0][1], &offset[1][2][1]};
    map<char, Dir> m = {{'L', Left}, {'F', Front}, {'R', Right}, {'B', Back}, {'U', Up}, {'D', Down}};
    double step = 0.08;
};
const int F = 0, B = 5, L = 1, R = 4, U = 2, D = 3; // front,back,left,right,up,down
// const int White = 0, Yellow = 5, Red = 1, Orange = 4, Green = 2, Blue = 3;
const char Color[6] = {'W', 'R', 'G', 'B', 'O', 'Y'};
string Command;
int step = 0;
bool isSrand = 0;
void init(char cube[][3][3]);
void random(char cube[][3][3], int N);
void setColor(unsigned short ForeColor, unsigned short BackGroundColor);
void rotate(char cube[][3][3], int mode, int direct);
void front(char cube[][3][3], int mode);
void left(char cube[][3][3], int mode);
void right(char cube[][3][3], int mode);
void back(char cube[][3][3], int mode);
void up(char cube[][3][3], int mode);
void down(char cube[][3][3], int mode);
void move(char cube[][3][3], const char *com);
void turn(char cube[][3][3]);
void Fturn(char cube[][3][3]);
void DownCross(char cube[][3][3]);
void DownMid(char cube[][3][3]);
void DownCorner(char cube[][3][3]);
void MidEdge(char cube[][3][3]);
void TopCross(char cube[][3][3]);
void TopCorner(char cube[][3][3]);
void TopLayerCorner(char cube[][3][3]);
void TopEdge(char cube[][3][3]);
void Solute(char cube[][3][3]); //!!Bug:turn 之后 Fturn方向改变 同理Fturn 之后 turn方向改变!!!!!!!!!!!!!!!!!!!!!!!!!!!
void initWinPos_Title(string Title, string newTitle = "");
string getPathName(string path);


int main(int argc, char *argv[])   
{
    RubikCube rubikcube;
    initgraph(Width, Height); // 创建窗口
    setbkcolor(RGB(35, 35, 35)); // 背景颜色：深灰
    setorigin(Width / 2, Height / 2); // 坐标原点为窗口中心
    BeginBatchDraw();                 // 开始批量绘图模式，提高绘图效率
    initWinPos_Title(getPathName(argv[0]), "3D-Rubik's-Cube [By MrBeanC]"); // 设置窗口名称
    POINT org;
    POINT now;
    bool flag = false;
    bool isBeginExecute = true;
    bool hasRand = false;
    char Cube[6][3][3];
    init(Cube);
    while (true)
    { 
        if (GetKey(VK_LBUTTON)) // 鼠标左键
        {
            if (!flag)
                GetCursorPos(&org), flag = true; 
        }
        else
            flag = false;
        if (flag)
        {
            GetCursorPos(&now); // 记录鼠标光标位置，并移动视角？
            rubikcube.rotateZ(-1.0 * (now.x - org.x));
            rubikcube.rotateY(-1.0 * (now.y - org.y));
            GetCursorPos(&org);
        }
        if (rubikcube.isExecuteOver() && !hasRand && GetKey(VK_RBUTTON)) // 在没有被打乱并且按下右键的时候，执行打乱命令
        { // Random
            hasRand = true;
            Command = "";
            random(Cube, 500);
            rubikcube.setRotateStep(PI / 2);
            rubikcube.bindCommand(Command);
        }
        if (rubikcube.isExecuteOver() && isBeginExecute && hasRand && GetKey(VK_RBUTTON)) // 当魔方没有旋转的时候&&是否已经开始了复原&&是否已经打乱&&按下鼠标右键
        { // Solve
            isBeginExecute = false;
            Command = ""; // 清空命令字符串，用于存储还原过程中生成的旋转命令
            Solute(Cube);
            rubikcube.setRotateStep(0.1); // 设置旋转步长为 0.1，用于逐步执行还原操作。较小的步长可以使动画更加平滑
            rubikcube.bindCommand(Command); // 函数的作用是将命令字符串绑定到 rubikcube 对象，以便逐步执行这些命令
        }
        rubikcube.execute(); // 连续读取命令并调取相应的旋转函数
        rubikcube.updateRotate(); // 更新魔方的旋转状态，结束的时候，设置为still，表示旋转结束
        rubikcube.draw(); // 绘制当前魔方的状态
        FlushBatchDraw(); // EasyX 图形库的 FlushBatchDraw 函数用于刷新批量绘图模式中的内容,确保所有绘制操作立即生效，而不是等待缓冲区自动刷新
        Sleep(1);         // Sleep 函数的参数是毫秒数，这里暂停 1 毫秒
        cleardevice();    // EasyX 图形库的 cleardevice 函数用于清除当前窗口的内容,为下一次绘制操作准备空白窗口
    }
    // 扫尾
    EndBatchDraw(); // 批量结束制图
    closegraph(); // 关闭图形窗口
    _getch(); 
    return 0;
}
void init(char cube[][3][3])
{
    Command = "";
    for (int i = 0; i < 6; i++) // initialize
        for (int j = 0; j < 3; j++)
            for (int k = 0; k < 3; k++)
                cube[i][j][k] = Color[i];
}
void random(char cube[][3][3], int N) // 随机打乱
{
    if (!isSrand)
        srand(time(NULL)), isSrand = 1;
    for (int i = 0; i < N; i++)
    {
        int rnd = rand() % 8;
        int rmod = (rand() % 2) * 2 - 1;
        switch (rnd)
        {
        case 0:
            front(cube, rmod);
            break;
        case 1:
            back(cube, rmod);
            break;
        case 2:
            up(cube, rmod);
            break;
        case 3:
            right(cube, rmod);
            break;
        case 4:
            left(cube, rmod);
            break;
        case 5:
            down(cube, rmod);
            break;
        case 6:
            turn(cube);
            break;
        case 7:
            Fturn(cube);
            break;
        }
        printf("\rRanding...\r");
    }
}
void setColor(unsigned short ForeColor, unsigned short BackGroundColor)
{
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);                     // 获取当前窗口句柄
    SetConsoleTextAttribute(handle, ForeColor + BackGroundColor * 0x10); // 设置颜色
}
void rotate(char cube[][3][3], int mode, int direct) // 旋转一个面
{
    char tCube[3][3]; // front
    memcpy(tCube, cube[direct], sizeof(tCube)); // 将 cube[direct] 的内容复制到 tCube 中，确保在旋转过程中不会丢失原始数据。
    if (mode != 1 && mode != -1)
        exit(-1);
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            cube[direct][i][j] = mode == 1 ? tCube[2 - j][i] : tCube[j][2 - i];
}
void front(char cube[][3][3], int mode) // 不仅旋转前面本身，还会更新与前面相邻的其他面的边缘块
{
    // mode == 1 ? printf("F") : printf("F\'"); step++;
    mode == 1 ? Command += "F " : Command += "F' "; // mode表示顺转还是反转
    step++; 
    rotate(cube, mode, F); // 内层检测mode合法性
    char rSide, dSide;
    if (mode == 1)
        for (int i = 0; i < 3; i++)
        {
            rSide = cube[R][i][0];
            cube[R][i][0] = cube[U][2][i];
            cube[U][2][i] = cube[L][2 - i][2];
            cube[L][2 - i][2] = cube[D][0][2 - i];
            cube[D][0][2 - i] = rSide;
        }
    else
        for (int i = 0; i < 3; i++)
        {
            dSide = cube[D][0][2 - i];
            cube[D][0][2 - i] = cube[L][2 - i][2];
            cube[L][2 - i][2] = cube[U][2][i];
            cube[U][2][i] = cube[R][i][0];
            cube[R][i][0] = dSide;
        }
}
void left(char cube[][3][3], int mode)
{
    // mode == 1 ? printf("L") : printf("L\'"); step++;
    mode == 1 ? Command += "L " : Command += "L' ";
    step++;
    rotate(cube, mode, L); // 内层检测mode合法性
    char fSide, dSide;
    if (mode == 1)
        for (int i = 0; i < 3; i++)
        {
            fSide = cube[F][i][0];
            cube[F][i][0] = cube[U][i][0];
            cube[U][i][0] = cube[B][2 - i][2];
            cube[B][2 - i][2] = cube[D][i][0];
            cube[D][i][0] = fSide;
        }
    else
        for (int i = 0; i < 3; i++)
        {
            dSide = cube[D][i][0];
            cube[D][i][0] = cube[B][2 - i][2];
            cube[B][2 - i][2] = cube[U][i][0];
            cube[U][i][0] = cube[F][i][0];
            cube[F][i][0] = dSide;
        }
}
void right(char cube[][3][3], int mode)
{
    // mode == 1 ? printf("R") : printf("R\'"); step++;
    mode == 1 ? Command += "R " : Command += "R' ";
    step++;
    rotate(cube, mode, R); // 内层检测mode合法性
    char bSide, dSide;
    if (mode == 1)
        for (int i = 0; i < 3; i++)
        {
            bSide = cube[B][i][0];
            cube[B][i][0] = cube[U][2 - i][2];
            cube[U][2 - i][2] = cube[F][2 - i][2];
            cube[F][2 - i][2] = cube[D][2 - i][2];
            cube[D][2 - i][2] = bSide;
        }
    else
        for (int i = 0; i < 3; i++)
        {
            dSide = cube[D][2 - i][2];
            cube[D][2 - i][2] = cube[F][2 - i][2];
            cube[F][2 - i][2] = cube[U][2 - i][2];
            cube[U][2 - i][2] = cube[B][i][0];
            cube[B][i][0] = dSide;
        }
}
void back(char cube[][3][3], int mode)
{
    // mode == 1 ? printf("B") : printf("B\'"); step++;
    mode == 1 ? Command += "B " : Command += "B' ";
    step++;
    rotate(cube, mode, B); // 内层检测mode合法性
    char lSide, dSide;
    if (mode == 1)
        for (int i = 0; i < 3; i++)
        {
            lSide = cube[L][i][0];
            cube[L][i][0] = cube[U][0][2 - i];
            cube[U][0][2 - i] = cube[R][2 - i][2];
            cube[R][2 - i][2] = cube[D][2][i];
            cube[D][2][i] = lSide;
        }
    else
        for (int i = 0; i < 3; i++)
        {
            dSide = cube[D][2][i];
            cube[D][2][i] = cube[R][2 - i][2];
            cube[R][2 - i][2] = cube[U][0][2 - i];
            cube[U][0][2 - i] = cube[L][i][0];
            cube[L][i][0] = dSide;
        }
}
void up(char cube[][3][3], int mode)
{
    // mode == 1 ? printf("U") : printf("U\'"); step++;
    mode == 1 ? Command += "U " : Command += "U' ";
    step++;
    rotate(cube, mode, U); // 内层检测mode合法性
    char rSide, fSide;
    if (mode == 1)
        for (int i = 0; i < 3; i++)
        {
            rSide = cube[R][0][2 - i];
            cube[R][0][2 - i] = cube[B][0][2 - i];
            cube[B][0][2 - i] = cube[L][0][2 - i];
            cube[L][0][2 - i] = cube[F][0][2 - i];
            cube[F][0][2 - i] = rSide;
        }
    else
        for (int i = 0; i < 3; i++)
        {
            fSide = cube[F][0][2 - i];
            cube[F][0][2 - i] = cube[L][0][2 - i];
            cube[L][0][2 - i] = cube[B][0][2 - i];
            cube[B][0][2 - i] = cube[R][0][2 - i];
            cube[R][0][2 - i] = fSide;
        }
}
void down(char cube[][3][3], int mode)
{
    // mode == 1 ? printf("D") : printf("D\'"); step++;
    mode == 1 ? Command += "D " : Command += "D' ";
    step++;
    rotate(cube, mode, D); // 内层检测mode合法性
    char rSide, bSide;
    if (mode == 1)
        for (int i = 0; i < 3; i++)
        {
            rSide = cube[R][2][i];
            cube[R][2][i] = cube[F][2][i];
            cube[F][2][i] = cube[L][2][i];
            cube[L][2][i] = cube[B][2][i];
            cube[B][2][i] = rSide;
        }
    else
        for (int i = 0; i < 3; i++)
        {
            bSide = cube[B][2][i];
            cube[B][2][i] = cube[L][2][i];
            cube[L][2][i] = cube[F][2][i];
            cube[F][2][i] = cube[R][2][i];
            cube[R][2][i] = bSide;
        }
}
void move(char cube[][3][3], const char *com) //"FRUR'U'F'"
{
    int m = 0;
    while (com[m] != '\0')
    {
        int mode = 1;
        if (com[m + 1] == '\'')
            mode = -1;
        switch (com[m])
        {
        case 'F':
            front(cube, mode);
            break;
        case 'L':
            left(cube, mode);
            break;
        case 'R':
            right(cube, mode);
            break;
        case 'B':
            back(cube, mode);
            break;
        case 'U':
            up(cube, mode);
            break;
        case 'D':
            down(cube, mode);
            break;
        default:
            exit(-2);
        }
        if (mode == -1)
            m++;
        m++;
    }
}
void turn(char cube[][3][3]) // 只是根据Y轴改变视角？？
{
    // printf("→");
    Command += "→ ";
    rotate(cube, 1, F);
    rotate(cube, -1, B);
    char rSide;
    for (int m = 0; m < 3; m++)
        for (int i = 0; i < 3; i++)
        {
            rSide = cube[R][i][m];
            cube[R][i][m] = cube[U][2 - m][i];
            cube[U][2 - m][i] = cube[L][2 - i][2 - m];
            cube[L][2 - i][2 - m] = cube[D][m][2 - i];
            cube[D][m][2 - i] = rSide;
        }
}
void Fturn(char cube[][3][3]) // 绕x水平轴旋转，只改变视角？？
{
    // printf("↓");
    Command += "↓ ";
    rotate(cube, 1, L);
    rotate(cube, -1, R);
    char fSide;
    for (int m = 0; m < 3; m++)
        for (int i = 0; i < 3; i++)
        {
            fSide = cube[F][i][m];
            cube[F][i][m] = cube[U][i][m];
            cube[U][i][m] = cube[B][2 - i][2 - m];
            cube[B][2 - i][2 - m] = cube[D][i][m];
            cube[D][i][m] = fSide;
        }
}
void DownCross(char cube[][3][3]) // 底层十字
{
    const char std = cube[F][1][1];                                                                      // standard color(central color)
    while (cube[F][0][1] != std || cube[F][1][0] != std || cube[F][2][1] != std || cube[F][1][2] != std) // not cross
    {
        for (int i = 0; i < 4; i++)
        {
            if (cube[F][0][1] == std && cube[F][1][0] == std && cube[F][2][1] == std && cube[F][1][2] == std)
                return;
            if (cube[L][0][1] == std || cube[R][0][1] == std || cube[B][0][1] == std)
            {
                while (cube[F][0][1] == std)
                    front(cube, 1);
                while (cube[F][0][1] != std)
                    up(cube, 1);
            }
            turn(cube);
        }
        for (int i = 0; i < 4; i++)
        {
            if (cube[F][0][1] == std && cube[F][1][0] == std && cube[F][2][1] == std && cube[F][1][2] == std)
                return;
            if (cube[U][0][1] == std || cube[U][2][1] == std)
            {
                while (cube[F][0][1] == std)
                    front(cube, 1);
                up(cube, 1);
                if (cube[U][1][0] == std)
                {
                    while (cube[F][1][0] == std)
                        front(cube, 1);
                    left(cube, 1);
                }
                if (cube[U][1][2] == std)
                {
                    while (cube[F][1][2] == std)
                        front(cube, 1);
                    right(cube, -1);
                }
            }
            turn(cube);
        }
    }
}
void DownMid(char cube[][3][3]) // 正确归位，与四个面的中心颜色对齐
{
    const char std = cube[F][1][1];
    while (cube[U][1][1] != cube[U][2][1] || cube[F][0][1] != std)
        front(cube, 1);
    move(cube, "UU");
    while (cube[R][1][0] != cube[R][1][1] || cube[F][1][2] != std)
        front(cube, 1);
    move(cube, "RR");
    while (cube[D][0][1] != cube[D][1][1] || cube[F][2][1] != std)
        front(cube, 1);
    move(cube, "DD");
    while (cube[L][1][2] != cube[L][1][1] || cube[F][1][0] != std)
        front(cube, 1);
    move(cube, "DDRRUU");
}
void DownCorner(char cube[][3][3])
{
    const char std = cube[F][1][1];
    char *Uc = &cube[U][1][1], *Rc = &cube[R][1][1];
    char *Ur = &cube[U][0][2], *Bul = &cube[B][0][0];
    char *Ru = &cube[R][0][2];
    for (int i = 0; i < 4; i++) // 整体向右旋转后执行重复操作即可!!!!!!!!!!!!!!!!!!!!!
    {
        if ((cube[F][0][2] != std || cube[U][2][2] != *Uc || cube[R][0][0] != *Rc) && (cube[F][0][2] == std || cube[U][2][2] == std || cube[R][0][0] == std))
        {
            while (*Ur == std || *Ru == std || *Bul == std)
                back(cube, 1);
            move(cube, "RBR'"); // 根据朝向改公式!!!!!!!!!!!!!
        }
        turn(cube);
    }
    for (int i = 0; i < 4; i++)
    {
        if (cube[F][0][2] == std && cube[U][2][2] == *Uc && cube[R][0][0] == *Rc)
        {
            turn(cube);
            continue;
        }
        while (!((*Ur == *Uc || *Ur == *Rc || *Ur == std) && (*Ru == *Uc || *Ru == *Rc || *Ru == std) && (*Bul == *Uc || *Bul == *Rc || *Bul == std)))
            back(cube, 1);
        if (*Ru == std)
            move(cube, "RBR'");
        else if (*Ur == std)
            move(cube, "BRB'R'");
        else
            move(cube, "RB'R'BBRBR'");
        turn(cube);
    }
}
void MidEdge(char cube[][3][3]) // 根据朝向改公式!!!!!!!!!!!!!
{
    char Bc = cube[B][1][1];
    for (int i = 0; i < 4; i++)
    {
        if ((cube[U][1][2] == cube[U][1][1] && cube[R][0][1] == cube[R][1][1]) || (cube[U][1][2] == Bc || cube[R][0][1] == Bc)) // Correct || Bc
        {
            turn(cube);
            continue;
        }
        while (cube[R][1][2] != Bc && cube[B][1][0] != Bc)
            back(cube, 1);
        move(cube, "R'U'RBRB'R'U");
        turn(cube);
    }
    char *Uc = &cube[U][1][1], *Rc = &cube[R][1][1], *Dc = &cube[D][1][1], *Lc = &cube[L][1][1];
    for (int i = 0; i < 4; i++)
    {
        if ((cube[U][1][2] == *Uc && cube[R][0][1] == *Rc) && (cube[R][2][1] == *Rc && cube[D][1][2] == *Dc) && (cube[D][1][0] == *Dc && cube[L][2][1] == *Lc) && (cube[L][0][1] == *Lc && cube[U][1][0] == *Uc)) // Correct
            break;
        while (cube[U][0][1] == Bc || cube[B][0][1] == Bc)
            back(cube, 1);
        while (cube[U][0][1] != *Uc)
            turn(cube), back(cube, 1); // bug???
        if (cube[B][0][1] == *Lc)
            turn(cube), move(cube, "R'U'RBRB'R'U");
        else
            move(cube, "BRB'R'B'U'BU");
    }
}
void TopCross(char cube[][3][3])
{
    Fturn(cube);
    Fturn(cube);
    const char std = cube[F][1][1];
    char *Fu = &cube[F][0][1], *Fl = &cube[F][1][0], *Fr = &cube[F][1][2], *Fd = &cube[F][2][1];
    while (!(*Fu == std && *Fl == std && *Fr == std && *Fd == std))
    {
        if (*Fu != std && *Fl != std && *Fr != std && *Fd != std)
            move(cube, "DRFR'F'D'");
        while (*Fl != std)
            front(cube, 1);
        if (*Fr != std)
        {
            if (*Fd == std)
                front(cube, 1);
            move(cube, "DRFR'F'D'");
        }
        else
            move(cube, "DRFR'F'D'");
    }
}
void TopCorner(char cube[][3][3])
{
    const char std = cube[F][1][1];
    char *ul = &cube[F][0][0], *ur = &cube[F][0][2], *dl = &cube[F][2][0], *dr = &cube[F][2][2];
    int cnt = 0;
    *ul == std ? cnt++ : 0;
    *ur == std ? cnt++ : 0;
    *dl == std ? cnt++ : 0;
    *dr == std ? cnt++ : 0;
    if (cnt == 4)
        return;
    while (true)
    {
        if (cnt == 0)
        {
            while (cube[U][2][0] != std || cube[U][2][2] != std)
                front(cube, 1);
        }
        else if (cnt == 1)
        {
            while (cube[F][0][2] != std)
                front(cube, 1);
        }
        else if (cnt == 2)
        {
            if (*ul == *ur || *ur == *dr || *dr == *dl || *dl == *ul)
                while (*ul != *dl)
                    front(cube, 1);
            else
                while (cube[U][2][2] != std && cube[R][0][2] != std)
                    front(cube, 1);
        }
        move(cube, "RF'F'R'F'RF'R'");
        cnt = 0;
        *ul == std ? cnt++ : 0;
        *ur == std ? cnt++ : 0;
        *dl == std ? cnt++ : 0;
        *dr == std ? cnt++ : 0;
        if ((cnt == 1 && cube[D][0][0] == std) || cnt == 4)
            break;
    }
    while (*ur != std)
        front(cube, 1);
    while (*ul != std || *ur != std || *dl != std || *dr != std) //??
        move(cube, "RF'F'R'F'RF'R'");
}
void TopLayerCorner(char cube[][3][3])
{
    char *Uc = &cube[U][1][1], *Rc = &cube[R][1][1], *Lc = &cube[L][1][1], *Dc = &cube[D][1][1];
    char *U1 = &cube[U][2][0], *U2 = &cube[U][2][2], *R1 = &cube[R][0][0], *R2 = &cube[R][2][0];
    char *L1 = &cube[L][0][2], *L2 = &cube[L][2][2], *D1 = &cube[D][0][0], *D2 = &cube[D][0][2];
    if (*U1 == *U2 && *R1 == *R2 && *L1 == *L2 && *D1 == *D2)
        return;
    if (*U1 != *U2 && *R1 != *R2 && *L1 != *L2 && *D1 != *D2)
        move(cube, "RU'RDDR'URDDRR");
    while (*D1 != *D2)
        front(cube, 1);
    move(cube, "RU'RDDR'URDDRR");
}
void TopEdge(char cube[][3][3])
{
    char *Uc = &cube[U][1][1], *Rc = &cube[R][1][1], *Lc = &cube[L][1][1], *Dc = &cube[D][1][1];
    char *Um = &cube[U][2][1], *Rm = &cube[R][1][0], *Lm = &cube[L][1][2], *Dm = &cube[D][0][1];
    while (cube[U][2][2] != *Uc)
        front(cube, 1);
    if (*Um != *Uc && *Rm != *Rc && *Lm != *Lc && *Dm != *Dc)
        move(cube, "RF'RFRFRF'R'F'RR");
    while (*Um != *Uc)
        turn(cube);
    while (*Rm != *Rc || *Lm != *Lc || *Dm != *Dc)
        move(cube, "RF'RFRFRF'R'F'RR");
}
void Solute(char cube[][3][3])
{
    Command = "";
    DownCross(cube);
    DownMid(cube);
    DownCorner(cube);
    MidEdge(cube);
    TopCross(cube);
    TopCorner(cube);
    TopLayerCorner(cube);
    TopEdge(cube);
}
void initWinPos_Title(string Title, string newTitle)
{
    HWND hwnd = NULL;
    while (!hwnd)
        hwnd = FindWindowA(NULL, Title.c_str()), Sleep(1); // FindWindowW Error
    int Screen_W = GetSystemMetrics(SM_CXSCREEN);          // Screen_X，获取屏幕宽度
    int Screen_H = GetSystemMetrics(SM_CYSCREEN);          // Screen_Y，高度
    MoveWindow(hwnd, Screen_W / 2 - Width / 2, Screen_H / 2 - Height / 2, Width, Height, true);
    if (newTitle != "")
        SetWindowTextA(hwnd, newTitle.c_str());
}
string getPathName(string path) // 处理文件路径等信息
{
    int pos = path.find_last_of('\\');
    if (pos != string::npos)
        path = path.substr(pos + 1);
    pos = path.find_last_of('.');
    return path.substr(0, pos);
}