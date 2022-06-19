#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <array>

#include <list>
#include <set>
#include <utility>

// constant
#define SIZE 15
#define DEPTH 4
#define INF 0x7EFFFFFF
#define WINNING_POINT 10000000

// utility
#define DEBUG 1
#define VALUE_FUNCTION 0
#define CHECK_INIT 1
#define CHECK_MM 0
#define CHECK_VP 0

// global variables
int player;
long long calculationCount = 0;
std::array<std::array<int, SIZE>, SIZE> board;

// non-member function
struct Point;
class State;
Point getWinningMove(State& state);
Point getNextMove(std::ofstream& fout, State& state);
int minimaxAB(State state, int depth, int alpha, int beta, bool maxPlayer, std::ofstream& fout);
int evaluate_cont_score(int dead, int cont, bool my_turn);
int evaluate_horizontal(const std::array<std::array<int, SIZE>, SIZE>& board, int myColor);
int evaluate_vertical(const std::array<std::array<int, SIZE>, SIZE>& board, int myColor);
int evaluate_diagonal(const std::array<std::array<int, SIZE>, SIZE>& board, int myColor);
int evaluate_point(const std::array<std::array<int, SIZE>, SIZE>& board, int x, int y, int color);
int getPointScore(int mid_cnt, int l_cnt, int r_cnt, int l_dead, int r_dead);

const float MY_POINT[]{
    WINNING_POINT,       // 0 FIVE         ooooo
    100000,              // 1 FOUR_LIVE   _oooo_
    500,               // 2 FOUR_DEAD1  _oooox
    1000,                // 3 THREE_LIVE  _ooo_
    100,                 // 4 THREE_DEAD1 _ooox
    100,                 // 5 TWO_LIVE    _oo_
    10,                  // 6 TWO_DEAD1   _oox
    10,                  // 7 ONE_LIVE    _o_ 
};

const float OPP_POINT[]{
    WINNING_POINT,       // 0 FIVE         ooooo
    100000,              // 1 FOUR_LIVE   _oooo_
    500,               // 2 FOUR_DEAD1  _oooox
    1000,                // 3 THREE_LIVE  _ooo_
    100,                 // 4 THREE_DEAD1 _ooox
    100,                 // 5 TWO_LIVE    _oo_
    10,                  // 6 TWO_DEAD1   _oox
    10,                  // 7 ONE_LIVE    _o_ 
};


enum SPOT_STATE {
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2
};

struct Point {
    // Can Add neighbor type amount?
    int x, y;
    int h; //heuristic value
	Point() : Point(-1, -1) {}
	Point(float x, float y) : x(x), y(y) {}
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
    void operator=(const Point& rhs){
        x = rhs.x; y = rhs.y;
        return;
    }
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
    int& operator[](const size_t n) {
        if (n == 0) return x;
        if (n == 1) return y;
    };
};

struct SET_POINT_CMP{
    bool operator()(const Point &lhs, const Point &rhs){
        if(lhs.x!=rhs.x) return lhs.x < rhs.x;
        if(lhs.y!=rhs.y) return lhs.y < rhs.y;
        return 0;
    }
};

struct SET_POINT_HEURISTIC_CMP{
    bool operator()(const Point &lhs, const Point &rhs){
        //return lhs.h >= rhs.h;
        if(lhs.x!=rhs.x) return lhs.x < rhs.x;
        if(lhs.y!=rhs.y) return lhs.y < rhs.y;
        return 0;
    }
};

class State{
public:
    State(std::array<std::array<int, SIZE>, SIZE> bd, int player){
        myCnt = 0;
        for(int i=0; i<SIZE; i++){
            for(int j=0; j<SIZE; j++){
                board[i][j] = bd[i][j];
                if(board[i][j]==player){
                    myCnt+=1;
                }
            }
        }
        myColor = player;
        //generatePossibleMoves();
    }
    State(State& state){
        for(int i=0; i<SIZE; i++){
            for(int j=0; j<SIZE; j++){
                board[i][j] = state.board[i][j];
            }
        }
        myColor = state.myColor;
        myCnt = state.myCnt;
        //generatePossibleMoves();
    }

    bool checkSurrounding(std::array<std::array<int, SIZE>, SIZE> bd, int i, int j){
        if(i>0 && i<SIZE-1){
            if(j>0 && j<SIZE-1){
                return bd[i-1][j-1]>0 || bd[i-1][j]>0 || bd[i-1][j+1]>0 || bd[i][j-1]>0 || bd[i][j+1]>0 || bd[i+1][j-1]>0 || bd[i+1][j]>0 || bd[i+1][j+1]>0;
            }
            else if(j==0){
                return bd[i-1][j]>0 || bd[i-1][j+1]>0 || bd[i][j+1]>0 || bd[i+1][j]>0 || bd[i+1][j+1]>0;
            }
            else{
                return bd[i-1][j]>0 || bd[i-1][j-1]>0 || bd[i][j-1]>0 || bd[i+1][j]>0 || bd[i+1][j-1]>0;
            }
        }
        else if(i == 0){
            if(j>0 && j<SIZE-1){
                return bd[i][j-1]>0 || bd[i][j+1]>0 || bd[i+1][j-1]>0 || bd[i+1][j]>0 || bd[i][j+1]>0;
            }
            else if(j==0){
                return bd[i+1][j]>0 || bd[i][j+1]>0 || bd[i+1][j+1]>0;
            }
            else{
                return bd[i+1][j]>0 || bd[i][j-1]>0 || bd[i+1][j-1]>0;
            }
        }
        else{
            if(j>0 && j<SIZE-1){
                return bd[i][j-1]>0 || bd[i-1][j-1]>0 || bd[i-1][j]>0 || bd[i-1][j+1]>0 || bd[i][j+1]>0;
            }
            else if(j==0){
                return bd[i-1][j]>0 || bd[i-1][j+1]>0 || bd[i][j+1]>0;
            }
            else{
                return bd[i][j-1]>0 || bd[i-1][j-1]>0 || bd[i-1][j]>0;
            }
        }
    }
    // std::set<Point, SET_POINT_HEURISTIC_CMP>
    std::list<Point> generatePossibleMoves(int color){
        std::list<Point> possibleMoves;
        std::list<Point> Fives;
        std::list<Point> myFours, oppFours;
        std::list<Point> myDFours, oppDFours;
        std::list<Point> myTwoThrees, oppTwoThrees;
        std::list<Point> myThrees, oppThrees;
        std::list<Point> myTwos, oppTwos;
        std::list<Point> others;
        for(int i=0; i<SIZE; i++){
            for(int j=0; j<SIZE; j++){
                if(board[i][j]>0) continue;
                if(checkSurrounding(board, i, j)){
                    //if(CHECK_VP) std::cout << "{" << i <<',' << j << "}, "; 
                    Point step(i,j);
                    int myPointScore = evaluate_point(board, i, j, color);
                    int oppPointScore = evaluate_point(board, i, j, 3-color);
                    if(CHECK_VP) std::cout << "myScore: " << myPointScore << " oppScore:: " << oppPointScore << '\n';
                    if(myPointScore >= MY_POINT[0]){
                        Fives.push_back(step);
                        break;
                    } else if(oppPointScore >= OPP_POINT[0]){
                        Fives.push_back(step);
                    } else if(myPointScore >= MY_POINT[1]){
                        myFours.push_back(step);
                    } else if(oppPointScore >= OPP_POINT[1]){
                        oppFours.push_back(step);
                    } else if(myPointScore >= MY_POINT[2]){
                        myDFours.push_back(step);
                    } else if(oppPointScore >= OPP_POINT[2]){
                        oppFours.push_back(step);
                    } else if(myPointScore >= 2*MY_POINT[3]){
                        myTwoThrees.push_back(step);
                    } else if(oppPointScore >= 2*OPP_POINT[3]){
                        oppTwoThrees.push_back(step);
                    } else if(myPointScore >= MY_POINT[3]){
                        myThrees.push_back(step);
                    } else if(oppPointScore >= OPP_POINT[3]){
                        oppThrees.push_back(step);
                    } else if(myPointScore >= MY_POINT[5]){
                        myTwos.push_back(step);
                    } else if(oppPointScore >= OPP_POINT[5]){
                        oppTwos.push_back(step);
                    } else{
                        others.push_back(step);
                    }
                    //possibleMoves.push_back(step);
                }
            }
        }
        // Immediate Win or Prevent Lose
        if(Fives.size()){
            return Fives;
        }
        // Live Four >> all
        if(color == player && myFours.size()) return myFours;
        if(color == 3-player && oppFours.size()) return oppFours;

        if(color == player){
            myTwoThrees.splice(myTwoThrees.end(), oppTwoThrees);
            myTwoThrees.splice(myTwoThrees.end(), myDFours);
            myTwoThrees.splice(myTwoThrees.end(), oppDFours);
            myTwoThrees.splice(myTwoThrees.end(), myThrees);
            myTwoThrees.splice(myTwoThrees.end(), oppThrees);
            myTwoThrees.splice(myTwoThrees.end(), myTwos);
            myTwoThrees.splice(myTwoThrees.end(), oppTwos);
            myTwoThrees.splice(myTwoThrees.end(), others);
            possibleMoves = myTwoThrees;
        }
        else{
            oppTwoThrees.splice(myTwoThrees.end(), myTwoThrees);
            oppTwoThrees.splice(myTwoThrees.end(), oppDFours);
            oppTwoThrees.splice(myTwoThrees.end(), myDFours);
            oppTwoThrees.splice(myTwoThrees.end(), oppThrees);
            oppTwoThrees.splice(myTwoThrees.end(), myThrees);
            oppTwoThrees.splice(myTwoThrees.end(), oppTwos);
            oppTwoThrees.splice(myTwoThrees.end(), myTwos);
            oppTwoThrees.splice(oppTwoThrees.end(), others);
            possibleMoves = oppTwoThrees;
        }
        //if(DEBUG) std::cout << '\n';
        return possibleMoves;
    }

    int getScore(bool my_turn, int color){ //cmd 0:normal 1:searchWin 2:preventLose
        int score = 0;
        int target = my_turn?color:3-color;
        //score += (evaluate_horizontal(my_turn)+evaluate_vertical(my_turn)+evaluate_diagonal(my_turn));
        score = (evaluate_horizontal(board, target)+evaluate_vertical(board, target)+evaluate_diagonal(board, target));
    
        return score;
    }

    void put_stone(int i, int j, bool my_turn){
        board[i][j] = my_turn ? myColor : 3-myColor;
    }

    void remove_stone(int i, int j){
        board[i][j] = 0;
    }

    int myColor;
    int myCnt;
    std::array<std::array<int, SIZE>, SIZE> board;
};


void read_board(std::ifstream& fin) {
    fin >> player;
    std::cout << player << '\n';
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
        }
    }
}

void write_valid_spot(std::ofstream& fout, State& state, int player) {
    //srand(time(NULL));
    int init_score = state.getScore(true, player);
    if(DEBUG) std::cout << "Initial Score: " << init_score << '\n';

    std::list<Point> possibleMoves = state.generatePossibleMoves(player);
    
    if(possibleMoves.size()==0){
        fout << SIZE/2 << " " << SIZE/2 << '\n';
        fout.flush();
        return;
    }

    Point move = getNextMove(fout, state);
    if(DEBUG) std::cout << "Final Move: {" << move[0] <<  "," << move[1] << "}\n";
    fout << move[0] << " " << move[1] << '\n';
    fout.flush();

    return;
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);

    State start(board, player);
    write_valid_spot(fout, start, player);

    fin.close();
    fout.close();
    return 0;
}


// Board Heuristic
int evaluate_cont_score(int dead, int cont, bool my_turn){
    int score = 0;
    if(dead==2 && cont<5) return score;
    if(my_turn){
        switch (cont){
            case 5:
                score += MY_POINT[0];
                break;
            case 4:
                // Live 4
                if(dead==0) score += MY_POINT[1];
                else if(dead==1) score += MY_POINT[2];
                break;
            case 3:
                if(dead==0) score += MY_POINT[3];
                else if(dead==1) score += MY_POINT[4];
                break;
            case 2:
                if(dead==0) score += MY_POINT[5];
                else if(dead==1) score += MY_POINT[6];
                break;
            case 1:
                if(dead==0) score += MY_POINT[7];
                break;
            default:
                break;
        }
    }
    else{
        switch (cont){
            case 5:
                score -= OPP_POINT[0];
                break;
            case 4:
                // Live 4
                if(dead==0) score -= OPP_POINT[1];
                else if(dead==1) score -= OPP_POINT[2];
                break;
            case 3:
                if(dead==0) score -= OPP_POINT[3];
                else if(dead==1) score -= OPP_POINT[4];
                break;
            case 2:
                if(dead==0) score -= OPP_POINT[5];
                else if(dead==1) score -= OPP_POINT[6];
                break;
            default:
                break;
        }            
    }
    return score;
}

int evaluate_horizontal(const std::array<std::array<int, SIZE>, SIZE>& board, int color){
    int score = 0;
    int dead = 2;
    int cont = 0;
    int target = -1;
    for(int i=0; i<SIZE; i++){
        for(int j=0; j<SIZE; j++){
            if(target==-1){
                if(board[i][j]==0){
                    dead = 1;
                }
                else{
                    target = board[i][j];
                    cont = 1;
                }
            }
            else if(board[i][j] == target){
                cont += 1;
            }
            else if(board[i][j] == 0){
                if(cont > 0){
                    dead -= 1;
                    score += evaluate_cont_score(dead, cont, target==color);
                    cont = 0;
                    dead = 1;
                    target = -1;
                }
                else{
                    target = -1;
                    dead = 1;
                }
            }
            else{
                // block by opponent
                if(cont > 0){
                    score += evaluate_cont_score(dead, cont, target==color);
                }
                cont = 1;
                target = 3-target;
                dead = 2;
            }
        }
        if(cont > 0){
            score += evaluate_cont_score(dead, cont, target==color);
        }
        cont = 0; dead = 2; target = -1;
    }
    return score;
}

int evaluate_vertical(const std::array<std::array<int, SIZE>, SIZE>& board, int color){
    int score = 0;
    int dead = 2;
    int cont = 0;
    int target = -1;
    for(int j=0; j<SIZE; j++){
        for(int i=0; i<SIZE; i++){
            if(target==-1){
                if(board[i][j]==0){
                    dead = 1;
                }
                else{
                    target = board[i][j];
                    cont = 1;
                }
            }
            else if(board[i][j] == target){
                cont += 1;
            }
            else if(board[i][j] == 0){
                if(cont > 0){
                    dead -= 1;
                    score += evaluate_cont_score(dead, cont, target==color);
                    cont = 0;
                    dead = 1;
                    target = -1;
                }
                else{
                    target = -1;
                    dead = 1;
                }
            }
            else{
                // block by opponent
                if(cont > 0){
                    score += evaluate_cont_score(dead, cont, target==color);
                }
                cont = 1;
                target = 3 - target;
                dead = 2;
            }
        }
        if(cont > 0){
            score += evaluate_cont_score(dead, cont, target==color);
        }
        cont = 0; dead = 2; target = -1;
    }
    return score;
}

int evaluate_diagonal(const std::array<std::array<int, SIZE>, SIZE>& board, int color){
    int score = 0;
    int dead = 2;
    int cont = 0;
    int target = -1;
    // top-left to bottom-right
    for(int k=1-SIZE; k < SIZE-1; k++){
        int begin = std::max(0, k);
        int end = std::min(SIZE-1, SIZE+k-1);
        for(int i=begin; i<=end; i++){
            int j = i - k;
            if(target==-1){
                if(board[i][j]==0){
                    dead = 1;
                }
                else{
                    target = board[i][j];
                    cont += 1;
                }
            }
            else if(board[i][j] == target){
                cont += 1;
            }
            else if(board[i][j] == 0){
                if(cont > 0){
                    dead -= 1;
                    score += evaluate_cont_score(dead, cont, target==color);
                    cont = 0;
                    dead = 1;
                    target = -1;
                }
                else{
                    target = -1;
                    dead = 1;
                }
            }
            else{
                // block by opponent
                if(cont > 0){
                    score += evaluate_cont_score(dead, cont, target==color);
                    
                }
                cont = 1;
                target = 3 - target;
                dead = 2;
            }
        }
        if(cont > 0){
            score += evaluate_cont_score(dead, cont, target==color);
        }
        cont = 0; dead = 2; target = -1;
    }
    // top-right to bottom-left
    for(int k=0; k<2*(SIZE-1); k++){
        int start = std::min(k, SIZE-1);
        int end = std::max(k-SIZE+1, 0);
        for(int j=start; j>=end; j--){
            int i = k-j;
            if(target==-1){
                if(board[i][j]==0){
                    dead = 1;
                }
                else{
                    target = board[i][j];
                    cont += 1;
                }
            }
            else if(board[i][j] == target){
                cont += 1;
            }
            else if(board[i][j] == 0){
                if(cont > 0){
                    dead -= 1;
                    score += evaluate_cont_score(dead, cont, target==color);
                    cont = 0;
                    dead = 1;
                    target = -1;
                }
                else{
                    target = -1;
                    dead = 1;
                }
            }
            else{
                // block by opponent
                if(cont > 0){
                    score += evaluate_cont_score(dead, cont, target==color);
                    
                }
                cont = 1;
                target = 3-target;
                dead = 2;
            }
        }
        if(cont > 0){
            score += evaluate_cont_score(dead, cont, target==color);
        }
        cont = 0; dead = 2; target = -1;
    }
    return score;
}


// Point Heuristic
int evaluate_point(const std::array<std::array<int, SIZE>, SIZE>& board, int x, int y, int color){
    int score = 0;
    int l_dead, r_dead;
    int mid_cnt, l_cnt, r_cnt;
    int empty;

    // horizontal
    l_dead = r_dead = 0; mid_cnt = 1; l_cnt = r_cnt = 0; empty = 0;
    for(int j=y-1; ; j--){

        if(j<0){
            l_dead = 1;
            break;
        }

        char c = board[x][j];
        if(c == color){
            if(empty == 0){
                mid_cnt += 1;
            }
            else{
                l_cnt += 1;
            }
        }
        else if(c == 0){
            if(empty == 0){
                empty = 1;
            }
            else{
                break;
            }
        }
        else if(c == 3-color){
            l_dead = 1;
            break;
        }  
    }
    empty = 0;
    for(int j=y+1; ; j++){

        if(j>=SIZE){
            r_dead = 1;
            break;
        }

        char c = board[x][j];
        if(c == color){
            if(empty == 0){
                mid_cnt += 1;
            }
            else{
                r_cnt += 1;
            }
        }
        else if(c == 0){
            if(empty == 0){
                empty = 1;
            }
            else{
                break;
            }
        }
        else if(c == 3-color){
            r_dead = 1;
            break;
        }  
    }
    if(CHECK_VP) std::cout << "{"<<x<<","<<y<<"} " << "mid: " <<mid_cnt << " l: " << l_cnt << " r: "<<r_cnt <<" empty:"<<empty<<" l_dead: "<<l_dead<< " r_dead: "<<r_dead<<'\n';
    score += getPointScore(mid_cnt, l_cnt, r_cnt, l_dead, r_dead);

    // vertical
    l_dead = r_dead = 0; mid_cnt = 1; l_cnt = r_cnt = 0; empty = 0;
    for(int i=x-1; ; i--){

        if(i<0){
            l_dead = 1;
            break;
        }

        char c = board[i][y];
        if(c == color){
            if(empty == 0){
                mid_cnt += 1;
            }
            else{
                l_cnt += 1;
            }
        }
        else if(c == 0){
            if(empty == 0){
                empty = 1;
            }
            else{
                break;
            }
        }
        else if(c == 3-color){
            l_dead = 1;
            break;
        }  
    }
    empty = 0;
    for(int i=x+1; ; i++){

        if(i>=SIZE){
            r_dead = 1;
            break;
        }

        char c = board[i][y];
        if(c == color){
            if(empty == 0){
                mid_cnt += 1;
            }
            else{
                r_cnt += 1;
            }
        }
        else if(c == 0){
            if(empty == 0){
                empty = 1;
            }
            else{
                break;
            }
        }
        else if(c == 3-color){
            r_dead = 1;
            break;
        }  
    }

    if(CHECK_VP) std::cout << "{"<<x<<","<<y<<"} " << "mid: " <<mid_cnt << " l: " << l_cnt << " r: "<<r_cnt <<" empty:"<<empty<<" l_dead: "<<l_dead<< " r_dead: "<<r_dead<<'\n';
    score += getPointScore(mid_cnt, l_cnt, r_cnt, l_dead, r_dead);

    // top-left to bottom-right
    l_dead = r_dead = 0; mid_cnt = 1; l_cnt = r_cnt = 0; empty = 0;
    for(int k=1; ; k++){
        int nx = x-k, ny = y-k;
        if(nx<0 || ny<0){
            l_dead = 1;
            break;
        }

        char c = board[nx][ny];
        if(c == color){
            if(empty == 0){
                mid_cnt += 1;
            }
            else{
                l_cnt += 1;
            }
        }
        else if(c == 0){
            if(empty == 0){
                empty = 1;
            }
            else{
                break;
            }
        }
        else if(c == 3-color){
            l_dead = 1;
            break;
        }  
    }
    empty = 0;
    for(int k=1; ; k++){
        int nx = x+k, ny = y+k;
        if(nx>=SIZE || ny>=SIZE){
            r_dead = 1;
            break;
        }

        char c = board[nx][ny];
        if(c == color){
            if(empty == 0){
                mid_cnt += 1;
            }
            else{
                r_cnt += 1;
            }
        }
        else if(c == 0){
            if(empty == 0){
                empty = 1;
            }
            else{
                break;
            }
        }
        else if(c == 3-color){
            r_dead = 1;
            break;
        }  
    }
    if(CHECK_VP) std::cout << "{"<<x<<","<<y<<"} " << "mid: " <<mid_cnt << " l: " << l_cnt << " r: "<<r_cnt <<" empty:"<<empty<<" l_dead: "<<l_dead<< " r_dead: "<<r_dead<<'\n';
    score += getPointScore(mid_cnt, l_cnt, r_cnt, l_dead, r_dead);

    // top-right to bottom-left
    l_dead = r_dead = 0; mid_cnt = 1; l_cnt = r_cnt = 0; empty = 0;
    for(int k=1; ; k++){
        int nx = x+k, ny = y-k;
        if(nx>=SIZE || ny<0){
            l_dead = 1;
            break;
        }

        char c = board[nx][ny];
        if(c == color){
            if(empty == 0){
                mid_cnt += 1;
            }
            else{
                l_cnt += 1;
            }
        }
        else if(c == 0){
            if(empty == 0){
                empty = 1;
            }
            else{
                break;
            }
        }
        else if(c == 3-color){
            l_dead = 1;
            break;
        }  
    }
    empty = 0;
    for(int k=1; ; k++){
        int nx = x-k, ny = y+k;
        if(nx<0 || ny>=SIZE){
            r_dead = 1;
            break;
        }

        char c = board[nx][ny];
        if(c == color){
            if(empty == 0){
                mid_cnt += 1;
            }
            else{
                r_cnt += 1;
            }
        }
        else if(c == 0){
            if(empty == 0){
                empty = 1;
            }
            else{
                break;
            }
        }
        else if(c == 3-color){
            r_dead = 1;
            break;
        }  
    }
    if(CHECK_VP) std::cout << "{"<<x<<","<<y<<"} " << "mid: " <<mid_cnt << " l: " << l_cnt << " r: "<<r_cnt <<" empty:"<<empty<<" l_dead: "<<l_dead<< " r_dead: "<<r_dead<<'\n';
    score += getPointScore(mid_cnt, l_cnt, r_cnt, l_dead, r_dead);

    return score;
}

int getPointScore(int mid_cnt, int l_cnt, int r_cnt, int l_dead, int r_dead){
    
    if(mid_cnt >= 5) return MY_POINT[0]; // WIN
    if(mid_cnt >= 4 && (l_dead==0&&r_dead==0 || l_cnt&&r_cnt || l_cnt&&!r_dead || r_cnt&&!l_dead)) return MY_POINT[0];
    if(l_cnt==0 && r_cnt==0){
        int dead = l_dead+r_dead;
        if(dead==0){
            switch(mid_cnt){
                case 4: return MY_POINT[1];
                case 3: return MY_POINT[3];
                case 2: return MY_POINT[5];
                case 1: return MY_POINT[7];
            }
        }
        else if(dead==1){
            switch(mid_cnt){
                case 4: return MY_POINT[2];
                case 3: return MY_POINT[4];
                case 2: return MY_POINT[6];
            }
        }
        return 0;
    }

    int cnt, dead;
    if(l_cnt>r_cnt) {
        cnt = l_cnt+mid_cnt;
        if(r_cnt==0){
            dead = l_dead+r_dead;
        }
        else{
            dead = l_dead;
        }
    }
    else{
        cnt = r_cnt+mid_cnt; 
        if(l_cnt==0){
            dead = l_dead+r_dead;
        }
        else{
            dead = r_dead;
        }
    }

    if(dead==0){
        switch (cnt){
            case 8:                     // _oooo_oooo_ 
            case 7: return MY_POINT[0]; // _oooo_ooo_
            case 6:                     // _ooo_ooo_
            case 5:                     // _ooo_oo_ 
            case 4: return MY_POINT[2]; // _oo_oo_ ...
            case 3: return MY_POINT[3]; // _oo_o_ ... !!!!!!!
            case 2: return MY_POINT[6]; // _o_o_
        }
    }
    else if(dead==1){
        switch (cnt){
            case 8: return MY_POINT[0]; // _oooo_oooox 
            case 7: return MY_POINT[1]; // _oooo_ooox ...
            case 6:                     // _ooo_ooox ...
            case 5:                     // _ooo_oox ...
            case 4: return MY_POINT[4]; // _oo_oox ...
            case 3: return MY_POINT[6]; // _oo_ox ...
            case 2: return MY_POINT[7]; // _o_ox
        }        
    }
    else{
        switch (cnt){
            case 8:                     // xoooo_oooox 
            case 7:                     // xoooo_ooox ...
            case 6:                     // xooo_ooox ...
            case 5:                     // xooo_oox ...
            case 4: return MY_POINT[2]; // xoo_oox ...
            case 3:                     // xoo_ox ...
            case 2: return 0;           // xo_ox
        }                
    }

    return 0;
}

Point getInitMove(State& state){
    bool win = false;
    bool preventLose = false;
    Point winningMove;
    Point preventLosingMove;
    int preventLosePoint = 0;
    std::list<Point> possibleMoves = state.generatePossibleMoves(state.myColor);
    for(auto move : possibleMoves){

        // Immediate Win
        state.put_stone(move[0], move[1], true);
        if(state.getScore(true, state.myColor) >= WINNING_POINT){
            winningMove = move;
            
            win = true;
            break;
        }
        // Prevent opponent to win
        // state.put_stone(move[0], move[1], false);
        // int LS = state.getScore(false, player);
        // if(LS >= WINNING_POINT && LS>preventLosePoint){
        //     preventLosingMove = move;
        //     preventLosePoint = LS;
        //     preventLose = true;
        // }
        state.remove_stone(move[0], move[1]);
    }

    if(win){
        if(CHECK_INIT) std::cout << "Immediate Win:{" << winningMove[0] << "," << winningMove[1] << "}\n";
        return winningMove;
    }
    else if(preventLose){
        if(CHECK_INIT) std::cout << "Prevent Win:{" << preventLosingMove[0] << "," << preventLosingMove[1] << "}\n";
        return preventLosingMove;
    }
    else{
        if(CHECK_INIT) std::cout << "No Initial Move\n";
        return Point();
    }
    
}

Point getNextMove(std::ofstream& fout, State& state){
    Point bestMove;
    int bestScore = -INF;
    Point InitMove;

    if(CHECK_MM) std::cout << "Begin minimaxing\n";
    std::list<Point> possibleMoves = state.generatePossibleMoves(player);
    int cnt = 0;
    for(auto move: possibleMoves){
        //if(cnt>=30) break;
        cnt++;
        State temp_state(state);
        //if(DEBUG) std::cout << "Move: {" << move[0]<< ',' << move[1] << "}";
        temp_state.put_stone(move[0], move[1], true);
        if(DEBUG) std::cout << "Move: {" << move[0]<< ',' << move[1] << "} ";
        int v = minimaxAB(temp_state, DEPTH-1, -INF, INF, false, fout);
        if(DEBUG) std::cout << " --> final score: " << v << '\n';
        if(v > bestScore){
            bestScore = v;
            bestMove = move;
        }
    }
    if(bestMove != Point(-1,-1)){
        if(DEBUG) std::cout << "BestScore: " << bestScore << '\n';
        if(DEBUG) std::cout << "BestMove: {" << bestMove[0] <<  "," << bestMove[1] << "}\n";
    }

    return bestMove;
}

int minimaxAB(State state, int depth, int alpha, int beta, bool maxPlayer, std::ofstream& fout){
    if(depth == 0){
        //if(CHECK_MM) std::cout << "Look for " << state.myColor << '\n';
        int finalScore = state.getScore(true, state.myColor);
        if(CHECK_MM) std::cout<< "Score: " << finalScore << "\n";
        return finalScore;
        
    }

    if(maxPlayer){
        //if(CHECK_MM) std::cout << "Maxmizing...\n";
        
        std::list<Point> possibleMoves = state.generatePossibleMoves(player);

        int v = -INF;
        
        for(auto move: possibleMoves){
            State temp_state(state);
            temp_state.put_stone(move[0], move[1], true);

            int minimaxValue = minimaxAB(temp_state, depth-1, alpha, beta, !maxPlayer, fout);
            // v := max(v, alphabeta)
            v = std::max(v, minimaxValue);
            // α := max(α, v)
            alpha = std::max(alpha, v);
            // prune
            if(beta <= alpha){
                //if(CHECK_MM) std::cout << "alpha pruned\n";
                break;
            }
        }
        if(CHECK_MM) std::cout<< "depth: "<<depth << " v: " << v<< "\n";
        return v;
    }
    else{
        //if(CHECK_MM) std::cout << "Minimizing...\n";
        
        std::list<Point> possibleMoves = state.generatePossibleMoves(3-player);

        int v = INF;
        
        for(auto move: possibleMoves){
            State temp_state(state);
            temp_state.put_stone(move[0], move[1], false);

            int minimaxValue = minimaxAB(temp_state, depth-1, alpha, beta, !maxPlayer, fout);
            // v := min(v, alphabeta)
            v = std::min(v, minimaxValue);
            // β := min(β, v)
            beta = std::min(beta, v);
            // prune
            if(beta <= alpha){
                //if(CHECK_MM) std::cout << "alpha pruned\n";
                break;
            }
        }
        if(CHECK_MM) std::cout<< "depth: "<<depth << " v: " << v<< "\n";
        return v;
    }
}