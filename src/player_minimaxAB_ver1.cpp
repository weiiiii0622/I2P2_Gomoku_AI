#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <array>

#include <set>
#include <utility>

// constant
#define SIZE 15
#define DEPTH 3
#define INF 0x7EFFFFFF
#define WINNING_POINT 1000000

// utility
#define DEBUG 1
#define VALUE_FUNCTION 0
#define CHECK_INIT 1
#define CHECK_MM 0

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

const float MY_POINT[]{
    5*WINNING_POINT,     // 0 FIVE         ooooo
    WINNING_POINT,       // 1 FOUR_LIVE   _oooo_
    100,                 // 2 FOUR_DEAD1  _oooox
    50000,               // 3 THREE_LIVE  _ooo_
    100,                 // 4 THREE_DEAD1 _ooox
    100,                 // 5 TWO_LIVE    _oo_
    10,                  // 6 TWO_DEAD1   _oox
    5,                   // 7 ONE_LIVE    _o_ 
};

const float OPP_POINT[]{
    5*WINNING_POINT,     // 0 FIVE         xxxxx
    5*WINNING_POINT,     // 1 FOUR_LIVE   _xxxx_
    5*WINNING_POINT,     // 2 FOUR_DEAD1  _xxxxo
    100000,              // 3 THREE_LIVE  _xxx_
    1000,                // 4 THREE_DEAD1 _xxxo
    120,                 // 5 TWO_LIVE    _xx_
    50,                  // 6 TWO_DEAD1   _xxo
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

    std::set<Point, SET_POINT_HEURISTIC_CMP> generatePossibleMoves(){
        std::set<Point, SET_POINT_HEURISTIC_CMP> possibleMoves;
        for(int i=0; i<SIZE; i++){
            for(int j=0; j<SIZE; j++){
                if(board[i][j]>0) continue;
                if(checkSurrounding(board, i, j)){
                    //if(DEBUG) std::cout << "{" << i <<',' << j << "}, "; 
                    Point step(i,j);
                    // board[i][j] = myColor;
                    // step.h = getScore(true, myColor);
                    // board[i][j] = 0;
                    possibleMoves.insert(step);
                }
            }
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

    std::set<Point, SET_POINT_HEURISTIC_CMP> possibleMoves = state.generatePossibleMoves();
    
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

int evaluate_point(const std::array<std::array<int, SIZE>, SIZE>& board, int x, int y, int color){

}

Point getInitMove(State& state){
    bool win = false;
    bool preventLose = false;
    Point winningMove;
    Point preventLosingMove;
    int preventLosePoint = 0;
    std::set<Point, SET_POINT_HEURISTIC_CMP> possibleMoves = state.generatePossibleMoves();
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
    //Point InitMove;
    Point InitMove = getInitMove(state);

    if(InitMove[0] != -1 && InitMove[1] != -1){
        bestMove = InitMove;
    }
    else{
        //if(CHECK_MM) std::cout << "Begin minimaxing\n";
        std::set<Point, SET_POINT_HEURISTIC_CMP> possibleMoves = state.generatePossibleMoves();
        int cnt = 0;
        for(auto move: possibleMoves){
            //if(cnt>=30) break;
            cnt++;
            State temp_state(state);
            //if(DEBUG) std::cout << "Move: {" << move[0]<< ',' << move[1] << "} score: " << move.h << '\n';
            temp_state.put_stone(move[0], move[1], true);

            int v = minimaxAB(temp_state, DEPTH-1, -INF, INF, false, fout);
            //if(DEBUG) std::cout << "Move: {" << move[0]<< ',' << move[1] << "} final score: " << v << '\n';
            if(v > bestScore){
                bestScore = v;
                bestMove = move;
            }
        }
        if(bestMove != Point(-1,-1)){
            if(DEBUG) std::cout << "BestScore: " << bestScore << '\n';
            if(DEBUG) std::cout << "BestMove: {" << bestMove[0] <<  "," << bestMove[1] << "}\n";
        }
    }

    return bestMove;
}

int minimaxAB(State state, int depth, int alpha, int beta, bool maxPlayer, std::ofstream& fout){
    if(depth == 0){
        if(CHECK_MM) std::cout << "Look for " << state.myColor << '\n';
        int finalScore = state.getScore(true, state.myColor);
        //if(CHECK_MM) std::cout<< "Score: " << finalScore << "\n";
        return finalScore;
        
    }

    if(maxPlayer){
        //if(CHECK_MM) std::cout << "Maxmizing...\n";
        
        std::set<Point, SET_POINT_HEURISTIC_CMP> possibleMoves = state.generatePossibleMoves();

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
        return v;
    }
    else{
        //if(CHECK_MM) std::cout << "Minimizing...\n";
        
        std::set<Point, SET_POINT_HEURISTIC_CMP> possibleMoves = state.generatePossibleMoves();

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
        return v;
    }
}