#include "tetris.h"

static struct sigaction act, oact;

int main() {
	int exit = 0;

	initscr();
	noecho();
	keypad(stdscr, TRUE);
	createRankList(); // 테트리스를 실행하고 rank.txt를 기반으로 연결리스트 생성
	srand((unsigned int)time(NULL));

	while (!exit) {
		clear();
		switch (menu()) {
		case MENU_PLAY: play(); break;
		case MENU_RANK: rank(); break;
		case MENU_RECOMMEND: recommendedPlay(); break; // 추천 시스템을 위한 선언
		case MENU_EXIT: exit = 1; break;
		default: break;
		}
	}

	endwin();
	system("clear");
	return 0;
}

long evalSize(Node* head) {
	//if(head->child == NULL) return 0;
	//return sizeof(Node) + (head->child * sizeof(int)) + evalSize(head->child);
	//return 0;
}

void InitTetris() {
	int i, j;
	for (j = 0; j < HEIGHT; j++)
		for (i = 0; i < WIDTH; i++)
			field[j][i] = 0;

	nextBlock[0] = rand() % 7;
	nextBlock[1] = rand() % 7;
	nextBlock[2] = rand() % 7; // 두 번째 next 블록 정보를 초기화
	blockRotate = 0;
	blockY = -1;
	blockX = WIDTH / 2 - 2;
	score = 0;
	gameOver = 0;
	timed_out = 0;

	DrawOutline();
	DrawField();
	//DrawBlock(blockY,blockX,nextBlock[0],blockRotate,' ');
	//DrawShadow(blockY, blockX, nextBlock[0], blockRotate); // 처음 시작할 때 첫 블럭의 그림자를 그리기 위해 추가
	DrawNextBlock(nextBlock);
	PrintScore(score);
	root = (Node*)malloc(sizeof(Node)); // root 노드를 동적 할당
	t_space += sizeof(Node); // 동적할당 했으므로 총 메모리 공간에 sizeof(Node)를 더함
	root->level = 0; // root 노드의 level은 0
	for (int h = 0; h < HEIGHT; h++) { // HEIGHT 만큼
		for (int w = 0; w < WIDTH; w++) { // WIDTH 만큼
			root->recField[h][w] = field[h][w]; // 반복문을 돌면서 field 정보를 root 노드에 복사
		}
	}
	time_t start, stop; // recommend 시간 측정하기 위한 변수
	start = time(NULL);  // 시작 시간
	modified_recommend(root); // 현재 블록이 놓일 곳 recommend
	stop = time(NULL); // tree 구성이 끝난 시간
	t_time += (double)difftime(stop, start); // 총 tree를 구성하는 시간에 더함
	DrawBlockWithFeatures(blockY, blockX, nextBlock[0], blockRotate); // 첫 블록의 그림자, 추천을 그려줌
}

void DrawOutline() {
	int i, j;
	/* 블럭이 떨어지는 공간의 태두리를 그린다.*/
	DrawBox(0, 0, HEIGHT, WIDTH);

	/* next block을 보여주는 공간의 태두리를 그린다.*/
	move(2, WIDTH + 10);
	printw("NEXT BLOCK");
	DrawBox(3, WIDTH + 10, 4, 8);
	DrawBox(10, WIDTH + 10, 4, 8); // 두 번째 nextblock을 그려주기 위한 박스를 추가로 그림

/* score를 보여주는 공간의 태두리를 그린다.*/
	move(17, WIDTH + 10); // score Box를 그리기 위해 좌표 이동
	printw("SCORE");
	DrawBox(18, WIDTH + 10, 1, 8); // score Box를 그리기 위해 좌표 수정
}

int GetCommand() {
	int command;
	command = wgetch(stdscr);
	// reccommend play일 경우에는 Q, q만 입력이 가능하다.
	if (recFlag) {
		switch (command) {
		case 'q':
		case 'Q':
			command = QUIT;
			break;
		default:
			command = NOTHING;
			break;
			return command; // 입력받은 command를 반환
		}
	}
	switch (command) {
	case KEY_UP:
		break;
	case KEY_DOWN:
		break;
	case KEY_LEFT:
		break;
	case KEY_RIGHT:
		break;
	case ' ':	/* space key*/
		/*fall block*/
		break;
	case 'q':
	case 'Q':
		command = QUIT;
		break;
	default:
		command = NOTHING;
		break;
	}
	return command;
}

int ProcessCommand(int command) {
	int ret = 1;
	int drawFlag = 0;
	switch (command) {
	case QUIT:
		ret = QUIT;
		break;
	case KEY_UP:
		if ((drawFlag = CheckToMove(field, nextBlock[0], (blockRotate + 1) % 4, blockY, blockX)))
			blockRotate = (blockRotate + 1) % 4;
		break;
	case KEY_DOWN:
		if ((drawFlag = CheckToMove(field, nextBlock[0], blockRotate, blockY + 1, blockX)))
			blockY++;
		break;
	case KEY_RIGHT:
		if ((drawFlag = CheckToMove(field, nextBlock[0], blockRotate, blockY, blockX + 1)))
			blockX++;
		break;
	case KEY_LEFT:
		if ((drawFlag = CheckToMove(field, nextBlock[0], blockRotate, blockY, blockX - 1)))
			blockX--;
		break;
	default:
		break;
	}
	if (drawFlag) DrawChange(field, command, nextBlock[0], blockRotate, blockY, blockX);
	return ret;
}

void DrawField() {
	int i, j;
	for (j = 0; j < HEIGHT; j++) {
		move(j + 1, 1);
		for (i = 0; i < WIDTH; i++) {
			if (field[j][i] == 1) {
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(".");
		}
	}
}


void PrintScore(int score) {
	move(19, WIDTH + 11); // score Box를 이동한 만큼 좌표 수정
	printw("%8d", score);
}

void DrawNextBlock(int* nextBlock) {
	int i, j;
	for (i = 0; i < 4; i++) {
		move(4 + i, WIDTH + 13);
		for (j = 0; j < 4; j++) {
			if (block[nextBlock[1]][0][i][j] == 1) {
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
		}
		// 두 번째 nextBlock 그리기
		move(11 + i, WIDTH + 13); // 첫번째 nextBlock Box 아래로 좌표 이동
		for (int j = 0; j < 4; j++) {
			// 해당 위치에 블럭이 색칠되어야 한다면
			if (block[nextBlock[2]][0][i][j] == 1) {
				// 해당 위치에 블럭을 그림
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			// 아니라면 공백을 그림
			else printw(" ");
		}
	}
}

void DrawBlock(int y, int x, int blockID, int blockRotate, char tile) {
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++) {
			if (block[blockID][blockRotate][i][j] == 1 && i + y >= 0) {
				move(i + y + 1, j + x + 1);
				attron(A_REVERSE);
				printw("%c", tile);
				attroff(A_REVERSE);
			}
		}

	move(HEIGHT, WIDTH + 10);
}

void DrawBox(int y, int x, int height, int width) {
	int i, j;
	move(y, x);
	addch(ACS_ULCORNER);
	for (i = 0; i < width; i++)
		addch(ACS_HLINE);
	addch(ACS_URCORNER);
	for (j = 0; j < height; j++) {
		move(y + j + 1, x);
		addch(ACS_VLINE);
		move(y + j + 1, x + width + 1);
		addch(ACS_VLINE);
	}
	move(y + j + 1, x);
	addch(ACS_LLCORNER);
	for (i = 0; i < width; i++)
		addch(ACS_HLINE);
	addch(ACS_LRCORNER);
}

void play() {
	int command;
	clear();
	act.sa_handler = BlockDown;
	sigaction(SIGALRM, &act, &oact);
	InitTetris();
	time_t start, stop; 
	double duration;
	start = time(NULL);
	do {
		// recommend play 모드일 때 블록이 빨리 내려가게 하기 위해 ualarm으로 수정
		if (timed_out == 0 && recFlag) {
			ualarm(200000, 0);
			timed_out = 1;
		}
		else if (timed_out == 0) {
			alarm(1);
			timed_out = 1;
		}
		command = GetCommand();
		if (ProcessCommand(command) == QUIT) {
			alarm(0);
			DrawBox(HEIGHT / 2 - 1, WIDTH / 2 - 5, 1, 10);
			move(HEIGHT / 2, WIDTH / 2 - 4);
			printw("Good-bye!!");
			refresh();
			getch();

			return;
		}
		stop = time(NULL); 
		duration = (double)difftime(stop, start); // 테트리스 게임이 지속되는 시간
		if (recFlag) { // recommend play 모드일 때
                        move(7, WIDTH + 22);
                        printw("score(t)  :  %d", score); // score(t) 출력
			move(6, WIDTH + 22);
			printw("time  :  %.4f", duration); // 총 게임시간 출력
			move(8, WIDTH + 22);
			printw("space(t)  :  %lld", t_space); // 총 메모리 공간 출력
			move(9, WIDTH + 22);
			printw("time(t)  : %.4f", t_time); // tree를 구성하는 소요시간 출력
			move(10, WIDTH + 22);
			if (t_time == 0) { // 
				printw("time(t) is 0"); // t_time이 0이면 INF가 뜨는데 이를 방지
			}
			else printw("score-efficiency  :  %.4f", score / t_time); // 시간효율성  출력
			move(11, WIDTH + 22);
			printw("space-efficiency  :  %10llf / 1e5", score / (long double)(t_space / 100000)); //공간 효율성  출력, 단위가 너무 작아 t_space를 임의로 나누어 계산함
                        
		}
	} while (!gameOver);

	alarm(0);
	getch();
	DrawBox(HEIGHT / 2 - 1, WIDTH / 2 - 5, 1, 10);
	move(HEIGHT / 2, WIDTH / 2 - 4);
	printw("GameOver!!");
	refresh();
	getch();
	newRank(score);
}

char menu() {
	printw("1. play\n");
	printw("2. rank\n");
	printw("3. recommended play\n");
	printw("4. exit\n");
	return wgetch(stdscr);
}

/////////////////////////첫주차 실습에서 구현해야 할 함수/////////////////////////
// 해당 위치로 이동할 수 있다면 1, 이동할 수 없다면 0 반환
int CheckToMove(char f[HEIGHT][WIDTH], int currentBlock, int blockRotate, int blockY, int blockX) {
	// user code
	// blockX, blockY로부터 4 * 4 위치 탐색
	for (int i = 0; i < 4; i++) { // 0 부터 3 까지 blockY  check
		for (int j = 0; j < 4; j++) { // 0 부터 3까지 blockX  check
			// 현재 블록의  칸이 존재하면
			if (block[currentBlock][blockRotate][i][j] == 1) {
				// blockY + i와 blockX + j가 0 보다 작으면 0 반환
				if (blockY + i < 0 || blockX + j < 0) return 0;
				// blockY + i가 HEIGHT 이상이거나 blockX + j가 WIDTH 이상이면 field를 벗어나므로 0 반환
				if (blockY + i >= HEIGHT || blockX + j >= WIDTH)return 0;
				// 필드에 이미 블록이 쌓여진 상태라면 0 반환
				if (f[blockY + i][blockX + j]) return 0;
			}
		}
	}
	// 이동할 수 있으므로 1 반환
	return 1;
}

// command에 따른 블록의 변화를 화면에 업데이트 해주는 함수. 이전 블록 정보를 지우고 새로운 블록을 그림.
void DrawChange(char f[HEIGHT][WIDTH], int command, int currentBlock, int blockRotate, int blockY, int blockX) {
	// user code

	//1. 이전 블록 정보를 찾는다. ProcessCommand의 switch문을 참조할 것
	switch (command) { // switch문 사용
	case KEY_RIGHT: // 오른쪽 방향키를 입력 받으면
		blockX--; // blockX 위치 -1
		break; // switch 끝냄
	case KEY_UP: // 위쪽 방향키를 입력 받으면
		blockRotate--; // blockRotate 회전 수 감소
		if (blockRotate < 0) blockRotate = 3; // blockRotate가 음수면 3 으로 업데이트
		break; // switch 끝냄
	case KEY_LEFT: // 왼쪽 방향키를 입력 받으면
		blockX++; // blockX 위치 +1
		break; // switch 끝냄
	case KEY_DOWN: // 아래쪽 방향키를 입력 받으면
		blockY--; // blockY 위치 -1
		break;	// switch 끝냄
	default: // default
		break; // switch 끝냄
	}
	//2. 이전 블록 정보를 지운다. DrawBlock함수 참조할 것.
	int shadowY = blockY; // 그림자의 y 좌표
	// CheckToMove() 함수를 호출하여 현재 블럭이 최대로 내려갈 수 있는 좌표(= 그림자의 좌표)를 구함
	// while문을 돌면서 아래로 내려갈 수 있는지 확인
	while (CheckToMove(f, currentBlock, blockRotate, shadowY + 1, blockX)) {
		shadowY += 1; // 현재 블럭이 내려갈 수 있으면 shadowY에 1 을 더함
	}
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (block[currentBlock][blockRotate][i][j] && i + blockY >= 0) {
				move(i + blockY + 1, j + blockX + 1); // i + bockY + 1, j + blockX + 1 로 커서 이동
				printw("%c", '.'); // 현재 좌표가 가리키는 화면에 . 출력(현재 블럭 지움)
				move(i + shadowY + 1, j + blockX + 1); // i + shadowY + 1, j + blockX + 1 로 커서 이동 (그림자 좌표)
				printw("%c", '.'); // 현재 좌표가 가리키는 화면에 . 출력(그림자 지움) 
			}
		}
	}
	//3. 새로운 블록 정보를 그린다. 
	switch (command) { // switch문 사용
	case KEY_RIGHT: // 오른쪽 방향키를 입력 받으면 
		blockX++; // blockX 위치 +1
		break; // switch 끝냄
	case KEY_UP: // 위쪽 방향키를 입력 받으면
		blockRotate++; // 회전수 +1
		if (blockRotate > 3) blockRotate = 0; // 만약 회전수가 3보다 크면0으로 업데이트
		break; // switch 끝냄
	case KEY_LEFT: // 왼쪽 방향키를 입력 받으면
		blockX--; // blockX 위치 -1
		break; // switch 끝냄
	case KEY_DOWN: // 아래 방향키를 입력 받으면
		blockY++; // blockY 위치 +1
		break; // switch 끝냄
	default: // default
		break; // switch 끝냄
	}
	//move(HEIGHT, WIDTH + 10);
		// 기존의 DrawBlock()을 DrawBlockWithFeatures()로 수정하여 현재 블럭과 그림자를 모두 그리도록 함
	DrawBlockWithFeatures(blockY, blockX, currentBlock, blockRotate); // 현재 블럭 정보를  해당 위치에 그려줌
	move(HEIGHT, WIDTH + 10); // 커서를 HEIGHT, WIDTH + 10 위치로 이동
}

void BlockDown(int sig) { // 블록을 매초 내려주는 함수
	// user code
	// recommend play 모드일 때 
	if (recFlag) {
		blockRotate = root->recBlockRotate; // 현재 블록 회전수를 추천 정보에 맞춤
		blockY = root->recBlockY; // 현재 블록의 Y 좌표를 추천 정보에 맞춤
		blockX = root->recBlockX; // 현재 블록의 X 좌표를 추천 정보에 맞춤
	}
	else {
		if (CheckToMove(field, nextBlock[0], blockRotate, blockY + 1, blockX)) { // 현재 블록이 아래 위치로 움직일 수 있다면
			blockY++; // blockY 위치 +1
			DrawChange(field, KEY_DOWN, nextBlock[0], blockRotate, blockY, blockX); // 아래로 내려간 블록을 그려줌
			timed_out = 0; // 시간을 0 으로 초기화
			return; // 움직임을 반영했다면 함수 종료
		}
	}

	if (blockY <= -1) { // blockY의 위치가 -1 이면
		// gameover 일 때 맨 윗줄에 블록을 채우기 위한 코드
		while (true) {
			bool flag = 1; // 블록이 해당 위치에 쌓일 수 있는지 4 * 4 크기로 확인
			for (int i = 3; i >= 0; i--) { // 3 to 0 범위의 HEIGHT 체크
				for (int j = 0; j < 4; j++) { // 0 to 3 범위의 WIDTH 체크
					if (block[nextBlock[0]][blockRotate][i][j] && i + blockY >= 0) { // 해당 블록의 칸이 존재할 때
						if (field[blockY + i][blockX + j]) flag = 0; // 이동할 수 없으면 flag = 0
					}
				}

			}
			if (!flag) blockY--; // flag가 0이면 이동할 수 없으므로 blockY 위치를 -1 해줌
			else {
				break; // 이동할 수 있으면 while문 종료
			}
		}
		for (int i = 0; i < 4; i++) { // 0 to 3 범위의 HEIGHT 체크
			for (int j = 0; j < 4; j++) { // 0 to 3 범위의 WIDTH 체크
				if (block[nextBlock[0]][blockRotate][i][j] && i + blockY >= 0) { // 해당 블록의 칸이 존재할 때
					move(i + blockY + 1, j + blockX + 1); // i + blockY + 1, j + blockX + 1 위치로 커서 이동
					// 테트리스 블럭을 색칠하여 출력
					attron(A_REVERSE);
					printw("%c", ' ');
					attroff(A_REVERSE);
				}
			}
		}
		gameOver = 1; // game이 끝남
		timed_out = 0; // 시간을 0으로 초기화
		return;
	}
	// 더이상 움직일 수 없으므로 현재 블록을 field에 업데이트 해줌
	score += AddBlockToField(field, nextBlock[0], blockRotate, blockY, blockX); // score에 AddBlockToField의 return 값을 더함

	// nextBlock 업데이트
	nextBlock[0] = nextBlock[1];
	nextBlock[1] = nextBlock[2]; // nextBlock[1]을 nextBlock[2]로 갱신
	nextBlock[2] = rand() % 7; // nextBlock[2] 초기화
// 다음 블록을 위한 변수 초기화
	blockRotate = 0;
	blockY = -1;
	blockX = WIDTH / 2 - 2;
	// 다음 블록을 화면에 출력
	DrawNextBlock(nextBlock);
	// 점수 정보를 업데이트하고 화면에 출력
	score += DeleteLine(field);
	PrintScore(score);
	// 테트리스 화면 출력	
	DrawField();
	// 시간을 0으로 초기화
	timed_out = 0;
	//강의자료 p26-27의 플로우차트를 참고한다.
		//if(recFlag) DrawBlock(blockY, blockX, nextBlock[0], blockRotate, ' '); 
	time_t start, stop;  // tree 구성하는데 걸린 시간을 위한 변수
	start = time(NULL); // tree 구성 시작
	if (root) free(root); // 이미 root 가 존재하면 root를 free함
	root = (Node*)malloc(sizeof(Node)); // root를 동적 할당
	root->level = 0; // root 노드의 level은 0
	for (int h = 0; h < HEIGHT; h++) { // HEIGHT 만큼
		for (int w = 0; w < WIDTH; w++) { // WIDTH 만큼
			root->recField[h][w] = field[h][w]; // 반복문을 돌면서 field 정보를 루트 노드에 복사한다.
		}
	}
	modified_recommend(root); // 현재 블록이 놓일 곳 recommend
	stop = time(NULL); // 끝나는 시간 
	t_time += (double)difftime(stop, start); // tree를 구성하는 시간을 더함 
	t_space += sizeof(Node); // 동적할당 했으므로 메모리 공간의 크기를 더함
	DrawBlockWithFeatures(blockY, blockX, nextBlock[0], blockRotate); // 블록 정보를 그려줌
}

// 현재 block을 field에 업데이트 함
// field와 닿은 면적 수를 구하여 touched * 10을 반환함
int AddBlockToField(char f[HEIGHT][WIDTH], int currentBlock, int blockRotate, int blockY, int blockX) {
	// user code	
	//Block이 추가된 영역의 필드값을 바꾼다.
	int touched = 0; // 닿은 면적 수
	for (int i = 0; i < 4; i++) { // 0 to 3 범위의 HEIGHT 체크
		for (int j = 0; j < 4; j++) { // 0 to 3 범위의 WIDTH 체크
			if (block[currentBlock][blockRotate][i][j]) { // 현재 블록의 칸이 존재하면
				if (blockY + i >= HEIGHT || blockX + j >= WIDTH) continue; // 좌표의 범위가 각각 HEIGHT, WIDTH 이상이라면 continue
				f[blockY + i][blockX + j] = 1; // 해당 좌표의 field를 1로 채워줌
								// 더이상 블럭이 움직일 수 없을 때 필드에 추가되는 바로 아래 위치의 field가 채워 있는 경우
								// 바닥에 쌓이거나, 블럭 아래가 채워져 있을 경우  touched++
				if (blockY + i + 1 == HEIGHT || f[blockY + i + 1][blockX + j] == 1) {
					touched++; // 조건을 만족하면 touched에 1 을 더함
				}
			}
		}
	}
	int ret_score = touched * 10; // 추가될 점수는 닿은 면적 * 10
	return ret_score; // 추가될 점수를 반환
}

// 다 채워진 가로줄을 지우고 위의 블록을 한 칸씩 내려줌
int DeleteLine(char f[HEIGHT][WIDTH]) {
	// user code
	int del_line = 0; // 지워진 line 수
	int flag = 1; // 가로줄이 다 채워져 있는지
	//1. 필드를 탐색하여, 꽉 찬 구간이 있는지 탐색한다.
	for (int i = 0; i < HEIGHT; i++) { // 가로줄 탐색
		flag = 1; // 탐색 전 flag를 1로 초기화 해줌
		for (int j = 0; j < WIDTH; j++) { // 가로줄 순회
			if (!f[i][j]) { // 만약 가로줄에 빈칸이 있으면
				flag = 0; // flag를 0으로 바꿔줌
				break; // for문 종료
			}
		}
		if (flag) { // 가로줄에 빈칸이 없다면
			for (int j = i - 1; j >= 0; j--) { // i-1 번째부터 0번째의 가로줄을 
				for (int k = 0; k < WIDTH; k++) { // 해당 가로줄의 모든 칸
					f[j + 1][k] = 0; // 아래 line을 0으로 초기화 함
					if (f[j][k]) f[j + 1][k] = f[j][k]; // 바로 위칸에 블록이 쌓여 있으면 아래로 내려줌
					f[j][k] = 0; // 내려준 칸은 0 으로 업데이트 함
				}

			}
			del_line++; // 지워진 line  +1
		}
	}
	// 지워진 line 수 * 지워진 line 수 * 100 반환
	return del_line * del_line * 100;
	//2. 꽉 찬 구간이 있으면 해당 구간을 지운다. 즉, 해당 구간으로 필드값을 한칸씩 내린다.
}

///////////////////////////////////////////////////////////////////////////

void DrawShadow(int y, int x, int blockID, int blockRotate) {
	// user code
	int shadowY = y; // 그림자의 y 좌표
	// 현재 블럭을 아래로 최대로 내릴 수 있는 위치(= 그림자의 위치)를 구한다.      
	// CheckToMove() 함수를 호출하여 현재 블럭이 (shadowY+1, x)좌표로 이동할 수 있는지 구함   
	while (CheckToMove(field, blockID, blockRotate, shadowY + 1, x)) {
		shadowY += 1; // 만약 아래로 내려갈 수 있다면 shadowY 좌표에 1 을 더함
	}
	DrawBlock(shadowY, x, blockID, blockRotate, '/'); // 위에서 구한 그림자의 좌표에 / 모양으로 그림자를 그려줌
	return; // DrawShadow 함수 종료
}

// 현재 block과 그림자를 함께 그려주는 함수이다. 
// DrawChange() 함수의 DrawBlock() 대신 DrawBlockWithFeatures() 함수를 호출하여 두 정보를 함께 그려준다.
void DrawBlockWithFeatures(int y, int x, int blockID, int blockRotate) {
	DrawRecommend(root->recBlockY, root->recBlockX, root->curBlockID, root->recBlockRotate); // 현재 블록의 추천 위치를 그려줌
	// recommend play 모드일 때 아래 정보는 그려주지 않는다.
	if (recFlag) return;
	DrawShadow(y, x, blockID, blockRotate); // 바닥에 현재 block의 그림자를 그려줌
	DrawBlock(y, x, blockID, blockRotate, ' '); // 현재 block 정보를 그려줌
	return; // DrawChange 함수 종료
}
// 게임을 시작할 때 linked list를 만들어 주는 함수
void createRankList() {
	// 목적: input파일인 "rank.txt"에서 랭킹 정보를 읽어들임. 읽어들인 정보로 랭킹 목록 생성
	//
// user code
	FILE* fp; // 파일 
	int i, j;

	fp = fopen("rank.txt", "a+"); // rank.txt 파일을 a+ 모드로 연다.

	int user_score; // 사용자의 점수
	char user_name[NAMELEN + 1]; // 사용자의 이름
	if (fscanf(fp, "%d", &score_number) != EOF) { // rank.txt에 데이터가 있으면
		head = NULL;
		NODE* cur = head; // 현재 우리가 보고 있는 노드
		for (i = 0; i < score_number; i++) { // 전체 데이터 개수 만큼 연결리스트 순회
			fscanf(fp, "%s %d", user_name, &user_score); // rank.txt에서 name, score 정보를 불러옴
			NODE* nxt = (NODE*)malloc(sizeof(NODE)); // 다음 노드를 동적 할당
			nxt->score = user_score; // 다음 노드의 score 정보 저장
			strcpy(nxt->name, user_name); // 다음 노드의 name 정보 저장
			nxt->link = NULL; // 다음 노드의 link를 null로 초기화
			if (head == NULL) { // 연결리스트가 존재하지 않으면
				head = nxt; // head를 nxt로 설정
				cur = head; // cur을 head로 초기화
			}
			else {
				cur->link = nxt; // 현재 노드에 다음 노드를 연결
				cur = cur->link; // 현재 노드를 다음 노드로 옮김
			}
		}
	}
	else { // rank.txt에 데이터가 없으면
		score_number = 0; // 전체 데이터 개수를 0으로 초기화
		head = NULL; // 연결리스트가 존재하지 않음
	}

	fclose(fp); // 파일을 닫음
}

void rank() {
	// user code
		// 목적: rank 메뉴를 출력하고 점수 순으로 X부터~Y까지 출력함
		// 1. 문자열 초기화
	int X = 1, Y = score_number, ch, i, j;
	clear();
	//2. printw()로 3개의 메뉴 출력
	move(0, 0);
	// 메뉴 출력
	printw("1. list ranks from X to Y\n");
	printw("2. list ranks by a specific name\n");
	printw("3. delete a specific rank\n");

	//3. wgetch()를 사용하여 변수 ch에 입력받은 메뉴번호 저장
	ch = wgetch(stdscr);

	//4. 각 메뉴에 따라 입력받을 값을 변수에 저장
	//4-1. 메뉴1: X, Y를 입력받고 적절한 input인지 확인 후 (X<=Y), X와 Y사이의 rank 출력
	if (ch == '1') { // 1을 입력 받으면
		printw("X: ");
		echo();
		scanw("%d", &X); // X를 입력받는다.
		printw("Y: ");
		scanw("%d", &Y); // Y를 입력받는다.
		// 화면 문구 출력
		printw("       name       |   score   \n");
		printw("------------------------------\n");
		if (X > Y) { // 만약 조건에 맞지 않는다면
			printw("search failure: no rank in the list\n"); // 에러 문구를 출력함       
		}
		else { // 범위를 충족한다면
			NODE* cur = head; // 첫 번째 노드부터 순회
			int count = 1; // 지금까지의 노드 개수
			for (int i = 0; i < score_number; i++) {
				if (X <= count && count <= Y) { // 현재 위치가 조건에 맞는다면
					printw(" %-17s| %-10d\n", cur->name, cur->score); // 현재 노드 정보 출력
				}
				cur = cur->link; // 다음 노드로 이동
				count++; // 지금까지의 노드 개수를 하나 늘려줌
			}
		}
		noecho();
	}
	//4-2. 메뉴2: 문자열을 받아 저장된 이름과 비교하고 이름에 해당하는 리스트를 출력
	else if (ch == '2') { // 2를 입력받으면
		char str[NAMELEN + 1]; // 찾을 문자열
		int check = 0; // linked list에서 문자열과 이름이 같은 랭킹 수
		printw("input the name: "); // 입력 문구 출력
		echo();
		scanw("%s", str); // 찾을 문자열을 입력받음
		// name, score frame 출력
		printw("       name       |   score   \n");
		printw("------------------------------\n");
		NODE* cur = head; // 첫번째 노드부터 탐색
		for (i = 1; i <= score_number; i++) { // 전체 랭킹 수 만큼 반복문을 돌면서 모든 노드 탐색
			if (strcmp(str, cur->name) == 0) { // 현재 보고 있는 노드의 이름 정보와 찾을 문자열이 같은 경우
				check++; // check를 1 늘림
			}
			cur = cur->link; // 다음 노드로 이동
		}
		cur = head; // cur을 첫 노드로 재설정
		if (check) { // linked list에 찾을 문자열이 존재하는 경우
			for (i = 1; i <= score_number; i++) { // 모든 노드 탐색
				if (strcmp(str, cur->name) == 0) { // 현재 보고 있는 노드의 이름 정보와 찾을 문자열이 같은 경우
					printw(" %-17s| %-10d\n", cur->name, cur->score); // 현재 노드의 랭킹 정보를 출력
				}
				cur = cur->link; // 다음 노드로 이동
			}
		}
		else { // linked list에 해당하는 정보가 없는 경우
			printw("\nsearch failure: no name in the list\n"); // 예외 문구 출력
		}
		noecho();
	}
	//4-3. 메뉴3: rank번호를 입력받아 리스트에서 삭제
	else if (ch == '3') { // 3을 입력받으면
		int num; // num 번째 랭킹 노드를 지움
		printw("input the rank: "); // 입력 문구 출력
		echo();
		scanw("%d", &num); // 지울 노드를 입력받음
		if (num < 1 || num > score_number) { // 만약 지울 노드가 범위에 맞지 않다면
			printw("\nsearch failure: the rank not in the list\n"); // 예외 문구 출력
		}
		else { // 지울 노드가 범위에 맞으면
			if (num > 1) { // 지울 노드가 첫번째가 아닐 때
				NODE* cur = head; // 지울 노드
				NODE* prev = NULL; // 지우기 바로 전 노드
				for (int i = 0; i < num - 1; i++) { // num-1 만큼 cur, prev 노드를 이동
					prev = cur; // prev를 다음 노드로 이동
					cur = cur->link; // cur을 다음 노드로 이동
				}
				prev->link = cur->link; // prev와 cur의 다음 노드를 연결
				free(cur); // 지울 노드인 cur을 free
			}
			else if (num == 1) { // 첫번째 노드를 지울 때
				NODE* tmp = head; // 임시로 head 주소를 저장
				head = head->link; // head를 다음 노드로 이동
				free(tmp); // 기존의 head를 free
			}
			printw("\nresult: the rank deleted\n"); // 삭제 완료 문구 출력
			score_number--; // 전체 랭킹 개수 하나 줄임
			writeRankFile(); // writeRankFile() 함수를 호출하여 rank.txt에 반영
		}
		noecho();
	}
	getch();
}

void writeRankFile() {
	// 목적: 추가된 랭킹 정보가 있으면 새로운 정보를 "rank.txt"에 쓰고 없으면 종료
// user code
	//1. "rank.txt" 연다.
	FILE* fp = fopen("rank.txt", "w"); // w모드로 rank.txt를 연다.
	//2. 랭킹 정보들의 수를 "rank.txt"에 기록
	fprintf(fp, "%d\n", score_number); // 전체 데이터 개수를 파일에 출력
	//3. 탐색할 노드가 더 있는지 체크하고 있으면 다음 노드로 이동, 없으면 종료
	NODE* cur = head; // 첫 번째 노드부터 정보 출력
	while (cur) { // cur이 NULL 값이 아니라면
		fprintf(fp, "%s %d\n", cur->name, cur->score); // rank.txt에 name, score 정보를 출력
		cur = cur->link; // 다음 노드로 이동
	}
	fclose(fp); // 파일 닫음
}

void newRank(int score) {
	// 목적: GameOver시 호출되어 사용자 이름을 입력받고 score와 함께 리스트의 적절한 위치에 저장
		// user code
	char user_name[NAMELEN + 1]; // 사용자 이름
	clear(); // 화면을 지움
	//1. 사용자 이름을 입력받음
	printw("your name: ");
	echo();
	scanw("%s", user_name); // 사용자의 이름을 입력받음
	noecho();
	//2. 새로운 노드를 생성해 이름과 점수를 저장, score_number가
	NODE* NewNode = (NODE*)malloc(sizeof(NODE)); // 새로운 노드 할당
	strcpy(NewNode->name, user_name); // 새로운 노드에 사용자 이름 저장
	NewNode->score = score; // 새로운 노드에 사용자 점수 저장
	NewNode->link = NULL; // 새로운 노드의 link는 NULL 값으로 설정
	NODE* cur = head; // 첫 번째 노드부터 탐색
	NODE* prev = NULL; // 새로운 노드를 삽입하는 위치의 바로 전 노드
	if (head == NULL) { // 연결리스트가 존재하지 않는다면
		head = NewNode; // head를 새로운 노드로 함
	}
	else {
		while (cur && score < cur->score) { // 새로운 노드가 삽입될 위치를 찾아줌
			prev = cur; // 이전 노드를 현재 노드로 갱신
			cur = cur->link; // 현재 노드를 다음 노드로 갱신
		}
		if (cur == head) { // 첫 번째 위치에 삽입 되어야 하면
			NewNode->link = head; // 새로운 노드의 다음 노드는 기존의 head
			head = NewNode; // head를 새로운 노드로 함
		}
		else if (cur == NULL) { // 마지막 위치에 삽입 되어야 하면
			prev->link = NewNode; // 이전 노드를 새로운 노드와 연결
		}
		else {
			prev->link = NewNode; // 이전 노드를 새로운 노드와 연결
			NewNode->link = cur; // 새로운 노드를 현재 노드와 연결
		}
	}
	score_number++; // 전체 데이터 개수를 하나 늘림
	writeRankFile(); // writeRankFile() 함수를 호출해 rank.txt 업데이트
}
// 추천하는 블록 위치를 R로 그리는 함수
void DrawRecommend(int y, int x, int blockID, int blockRotate) {
	// user code
	DrawBlock(y, x, blockID, blockRotate, 'R'); // 추천하는 위치에 해당하는 블록을 'R'로 출력함
}

// 다음에 어떤 위치에 블록을 놓아야 하는지 추천해주는 함수
int recommend(Node* root) {
	// user code
	int max = 0; // 미리 보이는 블럭의 추천 배치까지 고려했을 때 얻을 수 있는 최대 점수
	int score = 0; // 현재 노드에서 다음 블록까지 고려한 총 점수
	int idx = 0; // 현재 노드의 자식 노드 개수
	root->curBlockID = nextBlock[root->level]; // root의 블록 정보를 root->level을 이용해 저장
	root->accumulatedScore = -999; // root의 누적 점수를 음수로 초기화
	root->child = (Node**)malloc(sizeof(Node*)); // root의 자식 노드 배열을 동적 할당
	for (int i = 0; i < 4; i++) { // 블록의 회전 경우의 수
		for (int j = -4; j <= WIDTH + 4; j++) { // 블록이 놓일 수 있는 모든 가로 경우의 수
			if (!CheckToMove(root->recField, root->curBlockID, i, 0, j)) continue; // 만약 (0, j) 위치에 블록을 놓을 수 없는 경우에는 continue
			int recY = 0; // 추천되는 블록의 Y좌표
			score = 0; // 현재 노드의 점수를 0으로 초기화
			idx++; // 블록을 놓을 수 있다면 자식 노드 개수를 하나 늘림
			root->child = realloc(root->child, sizeof(Node*) * idx); // 늘어난 자식 노드 개수 만큼 root->child 배열을 realloc 해줌
			while (CheckToMove(root->recField, root->curBlockID, i, recY + 1, j)) recY++; // 블록을 최대한 아래로 내려줌
			root->child[idx - 1] = (Node*)malloc(sizeof(Node)); // root->child 배열의 노드를 동적 할당 함(root의 자식 노드)
			for (int h = 0; h < HEIGHT; h++) { // field의 높이 만큼
				for (int w = 0; w < WIDTH; w++) { // field의 너비 만큼
					root->child[idx - 1]->recField[h][w] = root->recField[h][w]; // 반복문을 돌면서 root의 field를 자식 노드의 field에 복사
				}
			}
			root->child[idx - 1]->level = root->level + 1; // 자식 노드의 level은 부모 노드의 level + 1
			score += AddBlockToField(root->child[idx - 1]->recField, root->curBlockID, i, recY, j); // score에 블록이 바닥에 닿을 떄의 점수를 더해줌
			score += DeleteLine(root->child[idx - 1]->recField); // score에 블록이 바닥에 닿을 때 지워진 line에 대한 점수를 더해줌

			if (root->child[idx - 1]->level < BLOCK_NUM) { // 자식 노드의 level이 설정한 BLOCK_NUM 보다 작으면
				score += recommend(root->child[idx - 1]); // recommend 함수를 재귀적으로 호출하여 자식의 max 점수를 root의 score에 더함
			}
			if (root->accumulatedScore < score) { // score가 root의 누적 점수보다 크다면
				root->recBlockY = recY; // root의 Y좌표를 현재 recY로 업데이트
				root->recBlockX = j; // root의 X좌표를 j로 업데이트
				root->recBlockRotate = i; // root의 회전 정보를 i로 업데이트
				root->accumulatedScore = score; // root의 누적 점수를 score로 업데이트
				max = score; // 얻을 수 있는 최대 점수를 score로 업데이트 함
			}
		}
	}
	// 자식 노드 배열의 노드를 free 함
	for (int i = 0; i < idx; i++) {
		free(root->child[i]);
	}
	free(root->child); // root의 자식 노드 배열을 free 함
	return max; // 추천을 통해 얻을 수 있는 최대 점수를 반환
}

// 효율을 고려하여 블록을 추천해주는 함수
int modified_recommend(Node* root) {
	// user code
	int max = 0; // 미리 보이는 블럭의 추천 배치까지 고려했을 때 얻을 수 있는 최대 점수
	int score = 0; // 현재 노드에서 다음 블록까지 고려한 총 점수
	int idx = 0; // 현재 노드의 자식 노드 개수
	root->recBlockY = -1; // 추천 블록의 y좌표를 -1로 초기화
	root->curBlockID = nextBlock[root->level]; // root의 블록 정보를 root->level을 이용해 저장
	root->accumulatedScore = -987654321; // root의 누적 점수를 음수로 초기화
	root->child = (Node**)malloc(sizeof(Node*)); // root의 자식 노드 배열을 동적 할당
	for (int i = 0; i < 4; i++) { // 블록의 회전 경우의 수
		if (duplicated_block(root->curBlockID, i) == 0) continue; // 만약 이전에 구한 블록과 모양이 똑같다면 새로운 노드를 생성하지 않음
		for (int j = -4; j <= WIDTH + 4; j++) { // 블록이 놓일 수 있는 모든 가로 경우의 수
			if (!CheckToMove(root->recField, root->curBlockID, i, 0, j)) continue; // 만약 (0, j) 위치에 블록을 놓을 수 없는 경우에는 continue
			int recY = -1; // 추천되는 블록의 Y좌표
			score = 0; // 현재 노드의 점수를 0으로 초기화
			idx++; // 블록을 놓을 수 있다면 자식 노드 개수를 하나 늘림
			t_space += sizeof(Node); // 동적 할당 했으므로 Node의 크기를 총 메모리 공간에 더함
			root->child = realloc(root->child, sizeof(Node*) * idx); // 늘어난 자식 노드 개수 만큼 root->child 배열을 realloc 해줌
			while (CheckToMove(root->recField, root->curBlockID, i, recY + 1, j)) recY++; // 블록을 최대한 아래로 내려줌
			root->child[idx - 1] = (Node*)malloc(sizeof(Node)); // root->child 배열의 노드를 동적 할당 함(root의 자식 노드)
			for (int h = 0; h < HEIGHT; h++) { // field의 높이 만큼
				for (int w = 0; w < WIDTH; w++) { // field의 너비 만큼
					root->child[idx - 1]->recField[h][w] = root->recField[h][w]; // 반복문을 돌면서 root의 field를 자식 노드의 field에 복사
				}
			}
			root->child[idx - 1]->level = root->level + 1; // 자식 노드의 level은 부모 노드의 level + 1
			// 닿은 면적
			root->child[idx - 1]->touched_width = AddBlockToField(root->child[idx - 1]->recField, root->curBlockID, i, recY, j); // score에 블록이 바닥에 닿을 떄의 점수를 더해줌
			// 삭제되면서 얻는 점수
			root->child[idx - 1]->deleted = DeleteLine(root->child[idx - 1]->recField); // score에 블록이 바닥에 닿을 때 지워진 line에 대한 점수를 더해줌
			// 최대 높이, 최소 높이, 빈공간 구하기
			root->child[idx - 1]->max_height = 0; // 최대 높이를 0으로 초기화
			root->child[idx - 1]->min_height = HEIGHT; // 최소 높이를 HEIGHT로 초기화
			root->child[idx - 1]->empty = 0; // 빈 공간을 0으로 초기화
			for (int w = 0; w < WIDTH; w++) {
				for (int h = 0; h < HEIGHT; h++) { // field의 위에서 아래로 내려오면서
					if (root->child[idx - 1]->recField[h][w] == 1) { // field에 블록이 쌓여있다면 
						if (root->child[idx - 1]->max_height < HEIGHT - h) { // 해당 조건을 만족하면
							root->child[idx - 1]->max_height = HEIGHT - h; // max_height 변경
						}
						if (root->child[idx - 1]->min_height > HEIGHT - h) { // 해당 조건을 만족하면
							root->child[idx - 1]->min_height = HEIGHT - h; // min_height 변경
						}
						for (int hh = h; hh < HEIGHT; hh++) { // 차있는 위치 아래 좌표를 탐색
							if (root->child[idx - 1]->recField[hh][w] == 0) { // 만약 빈공간이라면
								root->child[idx - 1]->empty++; // empty를 하나 늘려줌
							}
						}
						break;
					}
				}
			}


			// 가중치 계산
			/*if(root->child[idx-1]->max_height >= HEIGHT / 2 || root->child[idx-1]->max_height - root->child[idx-1]->min_height >= HEIGHT / 2){
					score += root->child[idx-1]->deleted * 100;
			}*/
			//if(blockY >= HEIGHT-3) score += 200;
			// 적절한 가중치를 곱해주어 score를 계산
			score += root->child[idx - 1]->touched_width; 
			score += root->child[idx - 1]->deleted * 8;
			score += root->child[idx - 1]->max_height * -30;
			score += root->child[idx - 1]->empty * -50;

			score += (root->child[idx - 1]->max_height - root->child[idx - 1]->min_height) * -100;
			if (root->child[idx - 1]->level < BLOCK_NUM) { // 자식 노드의 level이 설정한 BLOCK_NUM 보다 작으면
				score += modified_recommend(root->child[idx - 1]); // modified_recommend 함수를 재귀적으로 호출하여 자식의 max 점수를 root의 score에 더함
			}

			if (root->accumulatedScore < score) { // score가 root의 누적 점수보다 크다면
				root->recBlockY = recY; // root의 Y좌표를 현재 recY로 업데이트
				root->recBlockX = j; // root의 X좌표를 j로 업데이트
				root->recBlockRotate = i; // root의 회전 정보를 i로 업데이트
				root->accumulatedScore = score; // root의 누적 점수를 score로 업데이트
				max = score; // 얻을 수 있는 최대 점수를 score로 업데이트 함
			}
		}
	}
	// 자식 노드 배열의 노드를 free 함
	for (int i = 0; i < idx; i++) {
		free(root->child[i]);
	}
	free(root->child); // root의 자식 노드 배열을 free 함

	return max; // 추천을 통해 얻을 수 있는 최대 점수를 반환
}
// 블록을 회전했을 때 중복되는 모양을 확인해주기 위한 함수
int duplicated_block(int blockID, int blockRotate) {
	// 각 블록 모양과 회전 수를 고려했을 때 이전과 중복된다면 0 반환, 아니라면 1 반환
	if (blockID == 0) { 
		if (blockRotate == 2 || blockRotate == 3) {
			return 0;
		}
	}
	if (blockID == 4) {
		if (1 <= blockRotate) return 0;
	}
	if (blockID == 5) {
		if (2 <= blockRotate) return 0;
	}
	if (blockID == 6) {
		if (2 <= blockRotate) return 0;
	}
	return 1;
}

void recommendedPlay() {
	// user code
	recFlag = 1; // recommend play 모드일 때 recFlag를 1로 변경
	play(); // 테트리스 게임 진행
}
