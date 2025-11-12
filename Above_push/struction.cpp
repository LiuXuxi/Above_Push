#include<graphics.h>
#include<conio.h>
#include<stdio.h>
#include<windows.h>
#include<mmsystem.h>
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"EasyXa.lib")

//-------------------- 数据设计 --------------------

//-------------基础类型定义-------------
#define WALL '#'// 普通墙
#define BREAKABLE '~' //可破坏墙
#define BOX 'S'//箱子
#define BOOM '^'//炮弹
#define TARGET '.'//空目标点
#define PLAYER '@'//玩家
#define GOAL '*'//目标点（箱子需推入此处）
#define EMPTY ' '//空地
#define SUPERPOSITION '$'//玩家与目标点重合

//位置记录
typedef struct
{
	int x; //x行
	int y; //y列
}Position;

//-------------关于地图类型定义-------------

#define MAX_MAP_SIZE 8 //地图最大尺寸

// 地图结构体
typedef struct
{
	int level_id; // 关卡编号
	char Map_on[MAX_MAP_SIZE][MAX_MAP_SIZE];//地图
}Map_Level;


//地图存储 要求最外面都是不可破坏的墙。
char Map[4][MAX_MAP_SIZE][MAX_MAP_SIZE] = {
	{
		{WALL,WALL,WALL,WALL,WALL,WALL,WALL,WALL},
		{WALL,PLAYER,BOX,TARGET,WALL,WALL,WALL,WALL},
		{WALL,WALL,WALL,WALL,WALL,WALL,WALL,WALL},
		{WALL,WALL,WALL,WALL,WALL,WALL,WALL,WALL},
		{WALL,WALL,WALL,WALL,WALL,WALL,WALL,WALL},
		{WALL,WALL,WALL,WALL,WALL,WALL,WALL,WALL},
		{WALL,WALL,WALL,WALL,WALL,WALL,WALL,WALL},
		{WALL,WALL,WALL,WALL,WALL,WALL,WALL,WALL}
	},
	{
		{WALL,WALL,WALL,WALL,WALL,WALL,WALL,WALL},
		{WALL,EMPTY,PLAYER,EMPTY,EMPTY,EMPTY,EMPTY,WALL},
		{WALL,EMPTY,EMPTY,BOX,EMPTY,EMPTY,EMPTY,WALL},
		{WALL,WALL,WALL,EMPTY,WALL,EMPTY,EMPTY,WALL},
		{WALL,TARGET,WALL,EMPTY,WALL,BREAKABLE,BREAKABLE,WALL},
		{WALL,TARGET,BOX,EMPTY,EMPTY,WALL,EMPTY,WALL},
		{WALL,TARGET,BREAKABLE,EMPTY,EMPTY,BOX,EMPTY,WALL},
		{WALL,WALL,WALL,WALL,WALL,WALL,WALL,WALL}
	},
	{
		{'#','#','#','#','#','#','#','#'},
		{'#',' ','#','.','.','#',' ','#'},
		{'#','#','#',' ','.','#','#','#'},
		{'#','#',' ',' ','S','.','#','#'},
		{'#','#',' ','S',' ',' ','~','#'},
		{'#',' ','~','#','S','S',' ','#'},
		{'#',' ',' ','@',' ',' ',' ','#'},
		{'#','#','#','#','#','#','#','#'}
	},
	{
		{ '#', '#', '#', '#', '#', '#', '#', '#'},
		{ '#',' ',' ',' ', '#',' ',' ', '#'},
		{ '#',' ', '#', '~', '~','@',' ', '#'},
		{ '#',' ','S','S','S','S',' ', '#'},
		{ '#',' ', '.', '.', '.', '.', '#', '#'},
		{ '#', '#', '~', '#', '#',' ', '#', '#'},
		{ '#', '#',' ',' ',' ',' ', '#', '#'},
		{ '#', '#', '#', '#', '#', '#', '#', '#'}
	}
};




//-------------状态定义-------------

// 大炮状态定义
typedef struct
{
	int num;//炮弹数量
	Position pos;//炮弹当前位置
	char dir; // 移动方向（发射后不可改变）
}CannonState;

// 地图状态
typedef struct {
	Map_Level current_level;// 所选地图数据
	Position player_pos;// 玩家当前位置
	CannonState cannon;//大炮当前状态
}GameState;

GameState state;
//角色及箱子移动
/*
角色移动方向，wasd表示上左下右
*/
char dir;

//图片
IMAGE imgWall, imgBreakable, imgBox, imgTarget, imgPlayer, imgGoal, imgEmpty;
//-------------------- 数据设计 --------------------

//--------------------service --------------------
/*
负责人：空
	功能：
	init_level:初始化关卡数据
	根据关卡编号加载对应默认地图到map中
	根据关卡初始化人物位置和炮弹数。
	参数：无
	返回值：void
*/
void init_level();

/*
* 初始化游戏
*/
void init_game();

/*
	负责人：空
	功能：
	play：判断玩家是否可以向指定方向移动，如果可以，执行移动
	根据地图数据判断移动是否合法:

	目标为空地/空目标点：直接更新角色坐标 “player_pos"为移动后坐标(x±1, y±1)；
	目标为边界/墙：“player_pos"保持不变；
	此条先忽略：目标为幸运方块：好运：子弹数量加一
				   坏运：子弹数量减一
				   玩家位置更新
	目标为箱子：需进一步检测箱子移动后的坐标：
		若箱子移动后为空地/空目标点：同步更新“player_pos”和箱子坐标为移动后坐标；
		若箱子移动后为墙/其他箱子/幸运方块：”player_.pos"和箱子坐标均不更新（若为箱子则需重复此检测）
		箱子到达目的地之后显示为箱子，而不是目的地。
	返回值：
	0表示移动失败
	1表示移动成功
*/
int playerMove();

/*
	负责人：空
	功能：isWin：判断游戏是否胜利
	检查地图上有无目标点，有则未胜利，无则已胜利，进入胜利结算界面，调用函数winView()。
	参数：
	返回值：
	0表示未胜利
	1表示进入下一关
	2表示重置此关卡
	3表示退出游戏
	4表示返回菜单
*/
int isWin();

/*
	负责人：空
	功能：CannonLaunch:检查大炮是否能发射
	若发射,子弹数量减一
	检查CannonState.num是否等于0
	若等于0，不能发射
	若不等于0，成功发射
	返回值:
	0 表示发射失败
	1 表示发射成功
*/
int CannonLaunch();

/*
负责人：空
功能：发射炮弹
while(1){
	判断炮弹方向的下一位置处的元素
	①空地/空目标点：继续移动，调用game_display函数。
	②可破坏墙：停止移动并消失，可破坏墙体对应坐标更新为空地，跳出循环。
	③边界/不可破坏墙/箱子：停止移动并消失，边界/不可破坏墙/箱子对应坐标不更新，跳出循环。
}
参数：
返回值：void
*/
void BulletHit();

void musicPlay();

//--------------------service --------------------

//--------------------view --------------------

/*
负责人：空
功能：menuView:主菜单，展示选项，玩家可以在这里选择进入开始游戏、玩法介绍、团队介绍或退出游戏
while(1)(
	1.展示选项
	2.用户输入（使用ws移动光标）
	3.根据输入进行对应处理
	进入开始游戏：调用游戏界面函数gameview();
	进入团队介绍：调用团队介绍界面teamView();
	进入玩法介绍：调用RulesView();
	退出游戏：玩家按下esc键，调用exit(0)；
	参数：void
	返回值：void
*/
void menuView();

/*
负责人：空
功能：进入团队介绍画面
按下esc键回到主菜单，调用menuView();
参数:void
返回值：void
*/
void teamView();

/*
负责人：空
功能：进入规则介绍界面
按下esc键回到主菜单，调用menuView();
参数：void
返回值：void
*/
void RulesView();

/*
	负责人：空
	功能：进入胜利结算界面，展示“游戏结束”，玩家根据ws移动光标选择选项
	展示选项：
		退出游戏：
		进入下一关（这个选项需要额外判断，最后一关没有这个选项）
		重置此关卡：
		返回主菜单：
	参数：void
	返回值：int		1，2,3,4，分别表示进入下一关，重置此关卡,退出游戏,返回主菜单
*/
int winView();

/*
	负责人：张三
	功能：gameView：游戏界面整合，接受玩家信息
调用函数init(),对GameState进行初始化。
while(1)
{
	打印游戏界面（调用game_display;)
	接受玩家信息：
	若接受shift:,continue;
	若接受esc: 跳出循环，返回主菜单
	若接受空格，再判断下一个是w/s/a/d，调用CannonLaunch检验大炮是否能发射，如果不能则提示子弹为空，
		如果可以发射，则调用BulletHit()，提示发射成功。
	若接收玩家移动方向wasd输入
	执行移动操作（调用函数playerMove();)
	如果移动失败，则continue
	判断游戏是否胜利（调用函数isWin();)
			判断isWin()返回值：
				1：return 关卡号+1;
				2：再次调用init(map_level)进行重置关卡,continue;
				3：则清空state，break;
				4：调用exit(0);
				0：则 continue;
}
参数：
level_id：关卡结构体
返回值：int
*/
void gameView();
/*
* 游戏界面绘制
*/
void game_display();

/*
* 加载图片
*/
void loadAssets();

/*
* 通过所有关卡后界面展示
*/
void endView();
//--------------------view --------------------

int main()
{
	menuView();
	return 0;
}

void musicPlay() {

	int id = state.current_level.level_id;
	if (id == 0) {
		mciSendString(L"pause bgm", 0, 0, 0);
		mciSendString(L"pause bgm1", 0, 0, 0);
		mciSendString(L"pause bgm2", 0, 0, 0);
		mciSendString(L"pause bgm3", 0, 0, 0);
		mciSendString(L"open music\\LoFi-Chill.wav alias bgm type mpegvideo", 0, 0, 0);
		mciSendString(L"play bgm repeat", 0, 0, 0);
	}
	if (id == 1) {
		mciSendString(L"pause bgm", 0, 0, 0);
		mciSendString(L"pause bgm1", 0, 0, 0);
		mciSendString(L"pause bgm2", 0, 0, 0);
		mciSendString(L"pause bgm3", 0, 0, 0);
		mciSendString(L"open music\\Tobu-Candyland.wav alias bgm1 type mpegvideo", 0, 0, 0);
		mciSendString(L"play bgm1 repeat from 5500", 0, 0, 0);
	}
	if (id == 2) {
		mciSendString(L"pause bgm", 0, 0, 0);
		mciSendString(L"pause bgm1", 0, 0, 0);
		mciSendString(L"pause bgm2", 0, 0, 0);
		mciSendString(L"pause bgm3", 0, 0, 0);
		mciSendString(L"open music\\cure-light.wav alias bgm2 type mpegvideo", 0, 0, 0);
		mciSendString(L"play bgm2 repeat", 0, 0, 0);
	}

	if (id == 3) {
		mciSendString(L"pause bgm", 0, 0, 0);
		mciSendString(L"pause bgm1", 0, 0, 0);
		mciSendString(L"pause bgm2", 0, 0, 0);
		mciSendString(L"pause bgm3", 0, 0, 0);
		mciSendString(L"open music\\box.wav alias bgm3 type mpegvideo", 0, 0, 0);
		mciSendString(L"play bgm3 repeat", 0, 0, 0);
	}
}

void init_level()
{
	state.cannon.pos.x = -1;
	state.cannon.pos.y = -1;

	// 拷贝当前关卡地图
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			state.current_level.Map_on[i][j] = Map[state.current_level.level_id][i][j];
		}
	}

	// 设置子弹数
	state.cannon.num = 3;

	// 自动定位玩家（i=行, j=列）
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			if (state.current_level.Map_on[i][j] == PLAYER ||
				state.current_level.Map_on[i][j] == SUPERPOSITION) {
				state.player_pos.x = i;   
				state.player_pos.y = j;  
			}
		}
	}
}

void init_game() {
	state.current_level.level_id = 0;
}

int isWin()
{
	// 遍历地图，检查是否有箱子存在
	for (int i = 0; i < MAX_MAP_SIZE; i++) {
		for (int j = 0; j < MAX_MAP_SIZE; j++) {
			if (state.current_level.Map_on[i][j] == BOX) {
				// 如果找到箱子，说明游戏未胜利
				return 0;
			}
		}
	}

	// 如果没有箱子，说明胜利，返回1
	return 1;

}

int playerMove()
{
	int i = state.player_pos.x;
	int j = state.player_pos.y;

	switch (dir) {
	case 'W': case 'w':  // 上移
		if (state.current_level.Map_on[i - 1][j] == EMPTY || state.current_level.Map_on[i - 1][j] == TARGET) {
			// 普通移动
			if (state.current_level.Map_on[i][j] == SUPERPOSITION) state.current_level.Map_on[i][j] = TARGET;
			else state.current_level.Map_on[i][j] = EMPTY;

			if (state.current_level.Map_on[i - 1][j] == TARGET) state.current_level.Map_on[i - 1][j] = SUPERPOSITION;
			else state.current_level.Map_on[i - 1][j] = PLAYER;

			state.player_pos.x = i - 1;
			return 1;
		}
		else if (state.current_level.Map_on[i - 1][j] == BOX || state.current_level.Map_on[i - 1][j] == GOAL) {
			if (state.current_level.Map_on[i - 2][j] == EMPTY || state.current_level.Map_on[i - 2][j] == TARGET) {
				// 推箱子
				if (state.current_level.Map_on[i - 2][j] == TARGET)
					state.current_level.Map_on[i - 2][j] = GOAL;
				else
					state.current_level.Map_on[i - 2][j] = BOX;

				if (state.current_level.Map_on[i][j] == SUPERPOSITION)
					state.current_level.Map_on[i][j] = TARGET;
				else
					state.current_level.Map_on[i][j] = EMPTY;

				if (state.current_level.Map_on[i - 1][j] == GOAL)
					state.current_level.Map_on[i - 1][j] = SUPERPOSITION;
				else
					state.current_level.Map_on[i - 1][j] = PLAYER;

				state.player_pos.x = i - 1;
				return 1;
			}
		}
		break;

	case 'S': case 's':  // 下移
		if (state.current_level.Map_on[i + 1][j] == EMPTY || state.current_level.Map_on[i + 1][j] == TARGET) {
			if (state.current_level.Map_on[i][j] == SUPERPOSITION) state.current_level.Map_on[i][j] = TARGET;
			else state.current_level.Map_on[i][j] = EMPTY;

			if (state.current_level.Map_on[i + 1][j] == TARGET) state.current_level.Map_on[i + 1][j] = SUPERPOSITION;
			else state.current_level.Map_on[i + 1][j] = PLAYER;

			state.player_pos.x = i + 1;
			return 1;
		}
		else if (state.current_level.Map_on[i + 1][j] == BOX || state.current_level.Map_on[i + 1][j] == GOAL) {
			if (state.current_level.Map_on[i + 2][j] == EMPTY || state.current_level.Map_on[i + 2][j] == TARGET) {
				if (state.current_level.Map_on[i + 2][j] == TARGET)
					state.current_level.Map_on[i + 2][j] = GOAL;
				else
					state.current_level.Map_on[i + 2][j] = BOX;

				if (state.current_level.Map_on[i][j] == SUPERPOSITION)
					state.current_level.Map_on[i][j] = TARGET;
				else
					state.current_level.Map_on[i][j] = EMPTY;

				if (state.current_level.Map_on[i + 1][j] == GOAL)
					state.current_level.Map_on[i + 1][j] = SUPERPOSITION;
				else
					state.current_level.Map_on[i + 1][j] = PLAYER;

				state.player_pos.x = i + 1;
				return 1;
			}
		}
		break;

	case 'A': case 'a':  // 左移
		if (state.current_level.Map_on[i][j - 1] == EMPTY || state.current_level.Map_on[i][j - 1] == TARGET) {
			if (state.current_level.Map_on[i][j] == SUPERPOSITION) state.current_level.Map_on[i][j] = TARGET;
			else state.current_level.Map_on[i][j] = EMPTY;

			if (state.current_level.Map_on[i][j - 1] == TARGET) state.current_level.Map_on[i][j - 1] = SUPERPOSITION;
			else state.current_level.Map_on[i][j - 1] = PLAYER;

			state.player_pos.y = j - 1;
			return 1;
		}
		else if (state.current_level.Map_on[i][j - 1] == BOX || state.current_level.Map_on[i][j - 1] == GOAL) {
			if (state.current_level.Map_on[i][j - 2] == EMPTY || state.current_level.Map_on[i][j - 2] == TARGET) {
				if (state.current_level.Map_on[i][j - 2] == TARGET)
					state.current_level.Map_on[i][j - 2] = GOAL;
				else
					state.current_level.Map_on[i][j - 2] = BOX;

				if (state.current_level.Map_on[i][j] == SUPERPOSITION)
					state.current_level.Map_on[i][j] = TARGET;
				else
					state.current_level.Map_on[i][j] = EMPTY;

				if (state.current_level.Map_on[i][j - 1] == GOAL)
					state.current_level.Map_on[i][j - 1] = SUPERPOSITION;
				else
					state.current_level.Map_on[i][j - 1] = PLAYER;

				state.player_pos.y = j - 1;
				return 1;
			}
		}
		break;

	case 'D': case 'd':  // 右移
		if (state.current_level.Map_on[i][j + 1] == EMPTY || state.current_level.Map_on[i][j + 1] == TARGET) {
			if (state.current_level.Map_on[i][j] == SUPERPOSITION) state.current_level.Map_on[i][j] = TARGET;
			else state.current_level.Map_on[i][j] = EMPTY;

			if (state.current_level.Map_on[i][j + 1] == TARGET) state.current_level.Map_on[i][j + 1] = SUPERPOSITION;
			else state.current_level.Map_on[i][j + 1] = PLAYER;

			state.player_pos.y = j + 1;
			return 1;
		}
		else if (state.current_level.Map_on[i][j + 1] == BOX || state.current_level.Map_on[i][j + 1] == GOAL) {
			if (state.current_level.Map_on[i][j + 2] == EMPTY || state.current_level.Map_on[i][j + 2] == TARGET) {
				if (state.current_level.Map_on[i][j + 2] == TARGET)
					state.current_level.Map_on[i][j + 2] = GOAL;
				else
					state.current_level.Map_on[i][j + 2] = BOX;

				if (state.current_level.Map_on[i][j] == SUPERPOSITION)
					state.current_level.Map_on[i][j] = TARGET;
				else
					state.current_level.Map_on[i][j] = EMPTY;

				if (state.current_level.Map_on[i][j + 1] == GOAL)
					state.current_level.Map_on[i][j + 1] = SUPERPOSITION;
				else
					state.current_level.Map_on[i][j + 1] = PLAYER;

				state.player_pos.y = j + 1;
				return 1;
			}
		}
		break;
	}

	return 0;
}

void BulletHit()
{
	Position pos = state.player_pos;//获得大炮初始位置，也就是人的位置
	char kaka_dir = state.cannon.dir;//获得大炮移动方向

	while (1) {
		switch (kaka_dir) {//根据方向判断继续移动
		case 'W': pos.x--; break;
		case 'S': pos.x++; break;
		case 'A': pos.y--; break;
		case 'D': pos.y++; break;
		}
		if (state.current_level.Map_on[pos.x][pos.y] == EMPTY || state.current_level.Map_on[pos.x][pos.y] == TARGET) {//位置为空地、空目标点，炮弹继续走
			
			game_display();//调用显示函数
		}
		else if (state.current_level.Map_on[pos.x][pos.y] == BOX || state.current_level.Map_on[pos.x][pos.y] == WALL || state.current_level.Map_on[pos.x][pos.y] == GOAL) {//位置为不可破坏墙（边界）、箱子、已经有箱子的目标点
			break;//循环结束，炮弹消失
		}
		else if (state.current_level.Map_on[pos.x][pos.y] == BREAKABLE) {//位置等于可破坏墙，则墙的位置变成空地
			state.current_level.Map_on[pos.x][pos.y] = EMPTY;		
			break;//循环结束，炮弹消失
		}
	}
}

void menuView()
{

	// 初始化窗口
	initgraph(800, 880, EX_SHOWCONSOLE);
	setbkcolor(0xffffff);

	// 加载所有素材
	loadAssets();
	while (1) {
		musicPlay();
		init_game();
		IMAGE menuView;
		loadimage(&menuView, L"view\\menuView.png", 800, 880);
		putimage(0, 0, &menuView);
		FlushBatchDraw();

		ExMessage msg;
		getmessage(&msg, EX_KEY);
		//开始游戏
		if (msg.message == WM_KEYDOWN && msg.vkcode == 0x31) {
			cleardevice();
			gameView();
			cleardevice();
		}
		//游戏规则
		if (msg.message == WM_KEYDOWN && msg.vkcode == 0x32) {
			cleardevice();
			RulesView();
			cleardevice();
		}
		//团队介绍
		if (msg.message == WM_KEYDOWN && msg.vkcode == 0x33) {
			cleardevice();
			teamView();
			cleardevice();
		}
		//退出游戏
		if (msg.message == WM_KEYDOWN && msg.vkcode == 27) {
			closegraph();
			exit(0);
		}
	}
	


}

void teamView()
{
	cleardevice();

	//加载图片
	IMAGE teamView;
	loadimage(&teamView, L"view\\teamView.png");
	putimage(0, 0, &teamView);

	//提示按下esc键回到主菜单
	wchar_t esc[100] = { L"回到主菜单（esc键）" };
	setbkmode(TRANSPARENT);
	outtextxy(0, 0, esc);

	FlushBatchDraw();
	//接受esc键消息，调用menuView函数回到主菜单
	ExMessage msg;
	while (1) {
		getmessage(&msg, EX_KEY);
		if (msg.message == WM_KEYDOWN && msg.vkcode == 27) {
			return;;
		}
	}

}

void RulesView() {
	//创建一个窗口
	cleardevice();

	//加载图片
	IMAGE rulesView;
	loadimage(&rulesView, L"view\\RulesView.png");
	putimage(0, 0, &rulesView);

	//提示按下esc键回到主菜单
	wchar_t esc[100] = { L"回到主菜单（esc键）" };
	setbkmode(TRANSPARENT);
	outtextxy(0, 0, esc);
	FlushBatchDraw();
	//接受esc键消息，调用menuView函数回到主菜单
	ExMessage msg;
	while (1) {
		getmessage(&msg, EX_KEY);
		if (msg.message == WM_KEYDOWN && msg.vkcode == 27) {
			return;
		}
	}
}


int winView()
{

	IMAGE img;
	loadimage(&img, L"view\\winView.png", 800, 800);
	putimage(0, 0, &img);   // 先显示胜利画面
	FlushBatchDraw();
	int res = 5;
	while (1) {
		ExMessage msg;
		if (peekmessage(&msg, EX_KEY)) {  // 非阻塞获取按键
			if (msg.vkcode == '1') {      // 进入下一关
				res = 1;
				break;
			}
			else if (msg.vkcode == '2') { // 重置此关卡
				res = 2;
				break;
			}
			else if (msg.vkcode == '3') { // 退出游戏
				res = 3;
				break;
			}
			else if (msg.vkcode == '4') { // 返回菜单
				res = 4;
				break;
			}
		}
	}

	return res;
}

void gameView()
{
	//在此处完成代码
	init_level();
	while (1)
	{
		game_display();
		ExMessage msg;
		getmessage(&msg, EX_KEY);
		if (msg.message == WM_KEYDOWN)
		{
			//按下shift，重置游戏
			if (msg.vkcode == VK_SHIFT)
			{
				init_level();
				continue;
			}
			//按下esc，返回主菜单
			else if (msg.vkcode == VK_ESCAPE)
			{
				
				break;
			}
			//按下WASD，判断并执行对应操作
			int _playerMove;
			switch (msg.vkcode)
			{
			case'W':
				dir = 'W';
				_playerMove = playerMove();
				if (_playerMove == 0) {
					continue;
				}
				break;
			case'S':
				dir = 'S';
				_playerMove = playerMove();
				if (_playerMove == 0) {
					continue;
				}
				break;
			case'A':
				dir = 'A';
				_playerMove = playerMove();
				if (_playerMove == 0) {
					continue;
				}
				break;
			case'D':
				dir = 'D';
				_playerMove = playerMove();
				if (_playerMove == 0) {
					continue;
				}
				break;
			//按下F，发射大炮
			case'F':
				state.cannon.dir = dir;
				int CannonLaunch_result = CannonLaunch();
				if (CannonLaunch_result == 0)
				{
					//炮弹为零提示一次无炮弹
					mciSendString(L"close bgm5", 0, 0, 0);
					mciSendString(L"open music\\nofire.wav alias bgm5 type mpegvideo", 0, 0, 0);
					mciSendString(L"setaudio bgm5 volume to 1000", 0, 0, 0);
					mciSendString(L"play bgm5", 0, 0, 0);
					break;
				}
				else
				{
					//开炮音效
					mciSendString(L"open music\\fire.wav alias bgm4 type mpegvideo", 0, 0, 0);
					mciSendString(L"play bgm4 from 4500", 0, 0, 0);
					BulletHit();
					break;

				}
			}
		}
		//胜利判断
		int isWin_result = isWin();
		if (isWin_result == 0)
		{
			continue;
		}
		else
		{
			isWin_result = winView();
			if (isWin_result == 1) {
				state.current_level.level_id += 1;
				musicPlay();
				if (state.current_level.level_id == 4) {
					cleardevice();
					endView();
					return ;
				}
				else
				init_level();
			}
			else if (isWin_result == 2) {
				init_level();
				continue;
			}
			else if (isWin_result == 4) {
				return;
			}
			else if (isWin_result == 3) {
				closegraph();
				exit(0);
			}
		}
	}
}

void game_display() {
	const int cell = 100;

	// 双缓冲避免闪烁
	BeginBatchDraw();
	cleardevice();

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			int x = j * cell;
			int y = i * cell;

			switch (state.current_level.Map_on[i][j]) {
			case WALL:         putimage(x, y, &imgWall); break;
			case BREAKABLE:    putimage(x, y, &imgBreakable); break;
			case BOX:          putimage(x, y, &imgBox); break;
			case TARGET:       putimage(x, y, &imgTarget); break;
			case PLAYER:       putimage(x, y, &imgPlayer); break;
			case GOAL:         putimage(x, y, &imgGoal); break;
			case EMPTY:        putimage(x, y, &imgEmpty); break;

				// 玩家站在目标点上 -> 先画目标点，再画人物
			case SUPERPOSITION:
				putimage(x, y, &imgTarget);
				putimage(x, y, &imgPlayer);
				break;
			}
		}
	}

	wchar_t bulletText[64];
	settextcolor(0);
	settextstyle(50, 20, L"微软雅黑");
	swprintf_s(bulletText, L"子弹数目:%d    按shift重置", state.cannon.num);
	outtextxy(10, 800, bulletText);

	FlushBatchDraw();
}


int CannonLaunch() {
	if (state.cannon.num <= 0) {
		return 0; // 没有炮弹，发射失败
	}
	else {
		state.cannon.num--; // 减少炮弹数量
		return 1; // 发射成功
	}
}

void endView() {
	cleardevice();

	IMAGE endView;
	loadimage(&endView, L"view\\endView.png");
	putimage(0, 0, &endView);
	FlushBatchDraw();

	ExMessage msg;
	while (1) {
		getmessage(&msg, EX_KEY);
		if (msg.message == WM_KEYDOWN && msg.vkcode == 0x34) {
			return;
		}
	}

}



void loadAssets() {
	loadimage(&imgWall, L"assets\\wall.png", 100, 100);
	loadimage(&imgBreakable, L"assets\\nodeswall.jpg", 100, 100);
	loadimage(&imgBox, L"assets\\box.jpg", 100, 100);
	loadimage(&imgTarget, L"assets\\destination.png", 100, 100);
	loadimage(&imgPlayer, L"assets\\player.png", 100, 100);
	loadimage(&imgGoal, L"assets\\box.jpg", 100, 100); // 箱子在目标点，用同一张
	loadimage(&imgEmpty, L"assets\\blank.png", 100, 100);
}