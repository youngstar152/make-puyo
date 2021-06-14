#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <iostream>

//ぷよの色を表すの列挙型
//NONEが無し，RED,BLUE,..が色を表す
enum puyocolor { NONE, RED, BLUE, GREEN, YELLOW };

class PuyoArray
{
public:
    PuyoArray( ){           //コンストラクタ   
		data = NULL;   //盤面状態
		data_line = 0; //たて幅
		data_column = 0; //横幅
	}
	~PuyoArray(){         //デコンストラクタ
		Release();
	}

private:                //メンバ変数
	puyocolor *data;      //盤面状態
	unsigned int data_line; //たて幅
	unsigned int data_column; //横幅

//メモリ開放
	void Release()
	{
		if (data == NULL) {
			return;
		}

		delete[] data;
		data = NULL;
	}

public:
	//盤面サイズ変更
	void ChangeSize(unsigned int line, unsigned int column)
	{
		Release();

		//新しいサイズでメモリ確保
		data = new puyocolor[line*column];

		data_line = line;
		data_column = column;
	}

	//盤面の行数を返す
	unsigned int GetLine()
	{
		return data_line;	
	}

	//盤面の列数を返す
	unsigned int GetColumn()
	{
		return data_column;
	}

	//盤面の指定された位置の値を返す
	puyocolor GetValue(unsigned int y, unsigned int x)
	{
		if (y >= GetLine() || x >= GetColumn())
		{
			//引数の値が正しくない
			return NONE;
		}

		return data[y*GetColumn() + x];
	}

	//盤面の指定された位置に値を書き込む
	void SetValue(unsigned int y, unsigned int x, puyocolor value)
	{
		if (y >= GetLine() || x >= GetColumn())
		{
			//引数の値が正しくない
			return;
		}

		data[y*GetColumn() + x] = value;
	}
};

class PuyoArrayActive : public PuyoArray
{
public:
	PuyoArrayActive( ){
		puyorotate = 0;
	}

    void SetPuyoRotate(int new_puyorotate) {
        puyorotate = new_puyorotate;
    }

    int GetPuyoRotate() {
        return puyorotate;
    }	

private:
	int puyorotate;
};

class PuyoArrayStack : public PuyoArray
{

};

class PuyoControl
{
public:
//盤面に新しいぷよ生成
	bool GeneratePuyo(PuyoArrayActive &puyo,PuyoArrayStack &stack)
	{
        bool gene = false;
        if(stack.GetValue(0, 5) != NONE || stack.GetValue(0, 6) != NONE ){
            gene = true;
            return gene;
        }
		enum puyocolor newpuyo1, newpuyo2, newpuyo3, newpuyo4;  
		newpuyo1 = RED;
		newpuyo2 = BLUE;
		newpuyo3 = GREEN;
		newpuyo4 = YELLOW;
		switch(int ran1 =rand()%4)
		{
		case(0):
			puyo.SetValue(0, 5, newpuyo1);
			break;
		case(1):
			puyo.SetValue(0, 5, newpuyo2);
			break;
		case(2):
			puyo.SetValue(0, 5, newpuyo3);
			break;
		case(3):
			puyo.SetValue(0, 5, newpuyo4);
			break;
		}
		switch(int ran1 =rand()%4)
		{
		case(0):
			puyo.SetValue(0, 6, newpuyo1);
			break;
		case(1):
			puyo.SetValue(0, 6, newpuyo2);
			break;
		case(2):
			puyo.SetValue(0, 6, newpuyo3);
			break;
		case(3):
			puyo.SetValue(0, 6, newpuyo4);
			break;
		}	
		puyo.SetPuyoRotate(0);
        return gene;
	}

	//ぷよの着地判定．着地判定があるとtrueを返す
	bool LandingPuyo(PuyoArrayActive &puyo, PuyoArrayStack &stack)
	{
		bool landed = false;
		// 着地判定を行う
		for (int y = 0; y < puyo.GetLine(); y++)
		 {
            for (int x = 0; x < puyo.GetColumn(); x++) 
			{
                if (puyo.GetValue(y, x) != NONE && (y == puyo.GetLine() - 1 || stack.GetValue(y + 1, x) != NONE)) 
				{
                    landed = true;
                    break;
                }
            }
            if (landed) 
			{
                break;
            }
        }

        //着地判定したらpuyoからstackにぷよを移す
        if (landed)
		 {
            for (int y = 0; y < puyo.GetLine(); y++) 
			{
                for (int x = 0; x < puyo.GetColumn(); x++) 
				{
                    if (puyo.GetValue(y, x) != NONE) 
					{
                        //puyoの着地判定されたぷよをstackに移してから消す．
                        stack.SetValue(y, x, puyo.GetValue(y, x));
                        puyo.SetValue(y, x, NONE);
                    }
                }
            }
        }

        return landed;
	}

	//左移動
	bool MoveLeft(PuyoArrayActive &puyo, PuyoArrayStack &stack)
	{
		bool left = true;
        for (int y = 0; y < puyo.GetLine(); y++) 
        {
            for (int x = 0; x < puyo.GetColumn(); x++) 
            {
                if (puyo.GetValue(y, x) != NONE) 
                {
                    //左への移動先を確認
                    if (stack.GetValue(y, x - 1) != NONE) 
                    {
                        //ぷよがあって動けない
						left = false;
                        return left;
                    }
                }
            }
        }
		//一時的格納場所メモリ確保
		puyocolor *puyo_temp = new puyocolor[puyo.GetLine()*puyo.GetColumn()];

		for (int i = 0; i < puyo.GetLine()*puyo.GetColumn(); i++)
		{
			puyo_temp[i] = NONE;
		}

		//1つ左の位置にpuyoactiveからpuyo_tempへとコピー
		for (int y = 0; y < puyo.GetLine(); y++)
		{
			for (int x = 0; x < puyo.GetColumn(); x++)
			{
				if (puyo.GetValue(y, x) == NONE) {
					continue;
				}

				if (0 < x && puyo.GetValue(y, x - 1) == NONE)
				{
					puyo_temp[y*puyo.GetColumn() + (x - 1)] = puyo.GetValue(y, x);
					//コピー後に元位置のpuyoactiveのデータは消す
					puyo.SetValue(y, x, NONE);
				}
				else
				{
					puyo_temp[y*puyo.GetColumn() + x] = puyo.GetValue(y, x);
				}
			}
		}

		//puyo_tempからpuyoactiveへコピー
		for (int y = 0; y < puyo.GetLine(); y++)
		{
			for (int x = 0; x < puyo.GetColumn(); x++)
			{
				puyo.SetValue(y, x, puyo_temp[y*puyo.GetColumn() + x]);
			}
		}

		//一時的格納場所メモリ解放
		delete[] puyo_temp;
		return left;
	}

	//右移動
	bool MoveRight(PuyoArrayActive &puyo,PuyoArrayStack &stack)
	{	
		bool right=true;
        for (int y = 0; y < puyo.GetLine(); y++) 
        {
            for (int x = 0; x < puyo.GetColumn(); x++) 
            {
                if (puyo.GetValue(y, x) != NONE) 
                {
                    //右への移動先を確認
                    if (stack.GetValue(y, x + 1) != NONE) 
                    {
                        //ぷよがあって動けない
						right = false;
                        return right;
                    }
                }
            }
        }
		//一時的格納場所メモリ確保
		puyocolor *puyo_temp = new puyocolor[puyo.GetLine()*puyo.GetColumn()];

		for (int i = 0; i < puyo.GetLine()*puyo.GetColumn(); i++)
		{
			puyo_temp[i] = NONE;
		}

		//1つ右の位置にpuyoactiveからpuyo_tempへとコピー
		for (int y = 0; y < puyo.GetLine(); y++)
		{
			for (int x = puyo.GetColumn() - 1; x >= 0; x--)
			{
				if (puyo.GetValue(y, x) == NONE) {
					continue;
				}

				if (x < puyo.GetColumn() - 1 && puyo.GetValue(y, x + 1) == NONE)
				{
					puyo_temp[y*puyo.GetColumn() + (x + 1)] = puyo.GetValue(y, x);
					//コピー後に元位置のpuyoactiveのデータは消す
					puyo.SetValue(y, x, NONE);
				}
				else
				{
					puyo_temp[y*puyo.GetColumn() + x] = puyo.GetValue(y, x);
				}
			}
		}

		//puyo_tempからpuyoactiveへコピー
		for (int y = 0; y <puyo.GetLine(); y++)
		{
			for (int x = 0; x <puyo.GetColumn(); x++)
			{
				puyo.SetValue(y, x, puyo_temp[y*puyo.GetColumn() + x]);
			}
		}

		//一時的格納場所メモリ解放
		delete[] puyo_temp;
		return right;
	}

	//下移動
	bool MoveDown(PuyoArrayActive &puyo,PuyoArrayStack &stack)
	{
		bool down=true;

		for (int y = 0; y < puyo.GetLine(); y++) 
        {
            for (int x = 0; x < puyo.GetColumn(); x++) 
            {
                if (puyo.GetValue(y, x) != NONE) 
                {
                    //下への移動先を確認
                    if (stack.GetValue(y + 1, x) != NONE) 
                    {
                        //ぷよがあって動けない
                        down=false;
						return down;
                    }
                }
            }
        }

		//一時的格納場所メモリ確保
		puyocolor *puyo_temp = new puyocolor[puyo.GetLine()*puyo.GetColumn()];

		for (int i = 0; i < puyo.GetLine()*puyo.GetColumn(); i++)
		{
			puyo_temp[i] = NONE;
		}



		//1つ下の位置にpuyoactiveからpuyo_tempへとコピー
		for (int y = puyo.GetLine() - 1; y >= 0; y--)
		{
			for (int x = 0; x < puyo.GetColumn(); x++)
			{
				if (puyo.GetValue(y, x) == NONE) {
					continue;
				}

				if (y < puyo.GetLine() - 1 && puyo.GetValue(y + 1, x) == NONE)
				{
					puyo_temp[(y + 1)*puyo.GetColumn() + x] = puyo.GetValue(y, x);
					//コピー後に元位置のpuyoactiveのデータは消す
					puyo.SetValue(y, x, NONE);
				}
				else
				{
					puyo_temp[y*puyo.GetColumn() + x] = puyo.GetValue(y, x);
				}
			}
		}

		//puyo_tempからpuyoactiveへコピー
		for (int y = 0; y < puyo.GetLine(); y++)
		{
			for (int x = 0; x < puyo.GetColumn(); x++)
			{
				puyo.SetValue(y, x, puyo_temp[y*puyo.GetColumn() + x]);
			}
		}

		//一時的格納場所メモリ解放
		delete[] puyo_temp;
		return true;
	}

	//ぷよ消滅処理を全座標で行う
	//消滅したぷよの数を返す
	int VanishPuyo(PuyoArrayStack &stack)
	{
		int vanishednumber = 0;
		for (int y = 0; y < stack.GetLine(); y++)
		{
			for (int x = 0; x < stack.GetColumn(); x++)
			{
				vanishednumber += VanishPuyo(stack, y, x);
			}
		}

		return vanishednumber;
	}


	//回転
	//PuyoArrayActiveクラスのprivateメンバ変数として int puyorotate を宣言し，これに回転状態を記憶させている．
public:
	//puyorotateにはコンストラクタ及びGeneratePuyo関数で値0を代入する必要あり．
	void Rotate(PuyoArrayActive &puyo, PuyoArrayStack &stack)
	{
		//フィールドをラスタ順に探索（最も上の行を左から右方向へチェックして，次に一つ下の行を左から右方向へチェックして，次にその下の行・・と繰り返す）し，先に発見される方をpuyo1, 次に発見される方をpuyo2に格納
		puyocolor puyo1, puyo2;
		int puyo1_x = 0;
		int puyo1_y = 0;
		int puyo2_x = 0;
		int puyo2_y = 0;

		bool findingpuyo1 = true;
		for (int y = 0; y < puyo.GetLine(); y++)
		{
			for (int x = 0; x < puyo.GetColumn(); x++)
			{
				if (puyo.GetValue(y, x) != NONE)
				{
					if (findingpuyo1)
					{
						puyo1 = puyo.GetValue(y, x);
						puyo1_x = x;
						puyo1_y = y;
						findingpuyo1 = false;
					}
					else
					{
						puyo2 = puyo.GetValue(y, x);
						puyo2_x = x;
						puyo2_y = y;
					}
				}
			}
		}


		//回転前のぷよを消す
		puyo.SetValue(puyo1_y, puyo1_x, NONE);
		puyo.SetValue(puyo2_y, puyo2_x, NONE);


		//操作中ぷよの回転
		switch (puyo.GetPuyoRotate())
		{
		case 0:
			//回転パターン
			//RB -> R
			//      B
			//Rがpuyo1, Bがpuyo2
			if (puyo2_x <= 0 || puyo2_y >= puyo.GetLine() - 1 || stack.GetValue(puyo1_y + 1, puyo1_x) != NONE)	//もし回転した結果field_arrayの範囲外に出るなら回転しない,すでにぷよがある場合も回転しない
			{
				puyo.SetValue(puyo1_y, puyo1_x, puyo1);
				puyo.SetValue(puyo2_y, puyo2_x, puyo2);
				break;
			}

			//回転後の位置にぷよを置く
			puyo.SetValue(puyo1_y, puyo1_x, puyo1);
			puyo.SetValue(puyo2_y + 1, puyo2_x - 1, puyo2);
			//次の回転パターンの設定
			puyo.SetPuyoRotate(1);
			break;

		case 1:
			//回転パターン
			//R -> BR
			//B
			//Rがpuyo1, Bがpuyo2
			if (puyo2_x <= 0 || puyo2_y <= 0 || stack.GetValue(puyo1_y, puyo1_x-1) != NONE)	//もし回転した結果field_arrayの範囲外に出るなら回転しない,すでにぷよがある場合も回転しない
			{
				puyo.SetValue(puyo1_y, puyo1_x, puyo1);
				puyo.SetValue(puyo2_y, puyo2_x, puyo2);
				break;
			}

			//回転後の位置にぷよを置く
			puyo.SetValue(puyo1_y, puyo1_x, puyo1);
			puyo.SetValue(puyo2_y - 1, puyo2_x - 1, puyo2);

			//次の回転パターンの設定
			puyo.SetPuyoRotate(2);
			break;

		case 2:
			//回転パターン
			//      B
			//BR -> R
			//Bがpuyo1, Rがpuyo2
			if (puyo1_x >= puyo.GetColumn() - 1 || puyo1_y <= 0 || stack.GetValue(puyo1_y - 1, puyo1_x + 1) != NONE)	//もし回転した結果field_arrayの範囲外に出るなら回転しない,すでにぷよがある場合も回転しない
			{
				puyo.SetValue(puyo1_y, puyo1_x, puyo1);
				puyo.SetValue(puyo2_y, puyo2_x, puyo2);
				break;
			}

			//回転後の位置にぷよを置く
			puyo.SetValue(puyo1_y - 1, puyo1_x + 1, puyo1);
			puyo.SetValue(puyo2_y, puyo2_x, puyo2);

			//次の回転パターンの設定
			puyo.SetPuyoRotate(3);
			break;

		case 3:
			//回転パターン
			//B
			//R -> RB
			//Bがpuyo1, Rがpuyo2
			if (puyo1_x >= puyo.GetColumn() - 1 || puyo1_y >= puyo.GetLine() - 1 || stack.GetValue(puyo1_y + 1, puyo1_x + 1) != NONE)	//もし回転した結果field_arrayの範囲外に出るなら回転しない,すでにぷよがある場合も回転しない
			{
				puyo.SetValue(puyo1_y, puyo1_x, puyo1);
				puyo.SetValue(puyo2_y, puyo2_x, puyo2);
				break;
			}

			//回転後の位置にぷよを置く
			puyo.SetValue(puyo1_y + 1, puyo1_x + 1, puyo1);
			puyo.SetValue(puyo2_y, puyo2_x, puyo2);

			//次の回転パターンの設定
			puyo.SetPuyoRotate(0);
			break;

		default:
			break;
		}
	}

	//ぷよ消滅処理を座標(x,y)で行う
	//消滅したぷよの数を返す
	int VanishPuyo(PuyoArrayStack &stack, unsigned int y, unsigned int x)
	{
		//判定個所にぷよがなければ処理終了
		if (stack.GetValue(y, x) == NONE)
		{
			return 0;
		}


		//判定状態を表す列挙型
		//NOCHECK判定未実施，CHECKINGが判定対象，CHECKEDが判定済み
		enum checkstate{ NOCHECK, CHECKING, CHECKED };

		//判定結果格納用の配列
		enum checkstate *field_array_check;
		field_array_check = new enum checkstate[stack.GetLine()*stack.GetColumn()];

		//配列初期化
		for (int i = 0; i < stack.GetLine()*stack.GetColumn(); i++)
		{
			field_array_check[i] = NOCHECK;
		}

		//座標(x,y)を判定対象にする
		field_array_check[y*stack.GetColumn() + x] = CHECKING;

		//判定対象が1つもなくなるまで，判定対象の上下左右に同じ色のぷよがあるか確認し，あれば新たな判定対象にする
		bool checkagain = true;
		while (checkagain)
		{
			checkagain = false;

			for (int y = 0; y < stack.GetLine(); y++)
			{
				for (int x = 0; x < stack.GetColumn(); x++)
				{
					//(x,y)に判定対象がある場合
					if (field_array_check[y*stack.GetColumn() + x] == CHECKING)
					{
						//(x+1,y)の判定
						if (x < stack.GetColumn() - 1)
						{
							//(x+1,y)と(x,y)のぷよの色が同じで，(x+1,y)のぷよが判定未実施か確認
							if (stack.GetValue(y, x + 1) == stack.GetValue(y, x) && field_array_check[y*stack.GetColumn() + (x + 1)] == NOCHECK)
							{
								//(x+1,y)を判定対象にする
								field_array_check[y*stack.GetColumn() + (x + 1)] = CHECKING;
								checkagain = true;
							}
						}

						//(x-1,y)の判定
						if (x > 0)
						{
							if (stack.GetValue(y, x - 1) == stack.GetValue(y, x) && field_array_check[y*stack.GetColumn() + (x - 1)] == NOCHECK)
							{
								field_array_check[y*stack.GetColumn() + (x - 1)] = CHECKING;
								checkagain = true;
							}
						}

						//(x,y+1)の判定
						if (y < stack.GetLine() - 1)
						{
							if (stack.GetValue(y + 1, x) == stack.GetValue(y, x) && field_array_check[(y + 1)*stack.GetColumn() + x] == NOCHECK)
							{
								field_array_check[(y + 1)*stack.GetColumn() + x] = CHECKING;
								checkagain = true;
							}
						}

						//(x,y-1)の判定
						if (y > 0)
						{
							if (stack.GetValue(y - 1, x) == stack.GetValue(y, x) && field_array_check[(y - 1)*stack.GetColumn() + x] == NOCHECK)
							{
								field_array_check[(y - 1)*stack.GetColumn() + x] = CHECKING;
								checkagain = true;
							}
						}

						//(x,y)を判定済みにする
						field_array_check[y*stack.GetColumn() + x] = CHECKED;
					}
				}
			}
		}

		//判定済みの数をカウント
		int puyocount = 0;
		for (int i = 0; i < stack.GetLine()*stack.GetColumn(); i++)
		{
			if (field_array_check[i] == CHECKED)
			{
				puyocount++;
			}
		}

		//4個以上あれば，判定済み座標のぷよを消す
		int vanishednumber = 0;
		if (4 <= puyocount)
		{
			for (int y = 0; y < stack.GetLine(); y++)
			{
				for (int x = 0; x < stack.GetColumn(); x++)
				{
					if (field_array_check[y*stack.GetColumn() + x] == CHECKED)
					{
						stack.SetValue(y, x, NONE);

						vanishednumber++;
					}
				}
			}
		}

		//メモリ解放
		delete[] field_array_check;

		return vanishednumber;
	}

    bool DropPuyo(PuyoArrayStack &stack) 
    {
        bool droped = false;

        for (int y = (stack.GetLine() - 2); y >= 0; y--) 
        {
            for (int x = 0; x < stack.GetColumn(); x++) 
            {
                if (stack.GetValue(y, x) != NONE && stack.GetValue(y + 1, x) == NONE) 
                {
                    stack.SetValue(y + 1, x, stack.GetValue(y, x));
                    stack.SetValue(y, x, NONE);
                    droped = true;
                }
            }
        }
        return droped;
    }
};

	//表示
void Display(PuyoArrayActive &puyo, PuyoArrayStack &stack,int chain,int start,int maxchain,int score,int highscore)
{
	//落下中ぷよの色設定
	init_pair(0,COLOR_WHITE,COLOR_BLACK);
	init_pair(1,COLOR_RED,COLOR_BLACK);
	init_pair(2,COLOR_BLUE,COLOR_BLACK);
	init_pair(3,COLOR_GREEN,COLOR_BLACK);
	init_pair(4,COLOR_YELLOW,COLOR_BLACK);
	//落下中ぷよ表示
	for (int y = 0; y < puyo.GetLine(); y++)
	{
		for (int x = 0; x < puyo.GetColumn(); x++)
		{
			switch (puyo.GetValue(y, x))
			{
			case NONE:
				attrset(COLOR_PAIR(0));
				mvaddch(y, x, '.');
				break;
			case RED:
				attrset(COLOR_PAIR(1));
				mvaddch(y, x, 'R');
				break;
			case BLUE:
				attrset(COLOR_PAIR(2));
				mvaddch(y, x, 'B');
				break;
			case GREEN:
				attrset(COLOR_PAIR(3));
				mvaddch(y, x, 'G');
				break;
			case YELLOW:
				attrset(COLOR_PAIR(4));
				mvaddch(y, x, 'Y');
				break;
			default:
				mvaddch(y, x, '?');
				break;
			}
        }
    }

	// 着地済みのぷよの表示
    for (int y = 0; y < stack.GetLine(); y++)
	{
        for (int x = 0; x < stack.GetColumn(); x++)
		{
			switch (stack.GetValue(y, x))
			{
			case NONE:
				break;
			case RED:
				attrset(COLOR_PAIR(1));
				mvaddch(y, x, 'R');
				break;
			case BLUE:
				attrset(COLOR_PAIR(2));
				mvaddch(y, x, 'B');
				break;
			case GREEN:
				attrset(COLOR_PAIR(3));
				mvaddch(y, x, 'G');
				break;
			case YELLOW:
				attrset(COLOR_PAIR(4));
				mvaddch(y, x, 'Y');
				break;
			default:
				mvaddch(y, x, '?');
				break;
			}
			
		}
	}   


	//情報表示
    attrset(COLOR_PAIR(0));
	int count = 0;
	for (int y = 0; y < stack.GetLine(); y++)
	{
		for (int x = 0; x < stack.GetColumn(); x++)
		{
			if (stack.GetValue(y, x) != NONE)
			{
				count++;
			}
		}
	}
	//盤面サイズの表示と現在のぷよの数を表示
	char msg[256];
	sprintf(msg, "Field: %d x %d, Puyo number: %03d", puyo.GetLine(), puyo.GetColumn(), count);
	mvaddstr(2, COLS - 35, msg);

	//点数詳細
	sprintf(msg, "vanish:100point, chain:150point");
    mvaddstr(4, COLS - 35, msg);

    //連鎖数, 消えたぷよの数を表示
	attrset(COLOR_PAIR(3));
    sprintf(msg, "Total-vanish: %03d number", chain);
    mvaddstr(6, COLS - 35, msg);

    //最大連鎖数の表示
    sprintf(msg, "Max-Chain: %03d number", maxchain);
    mvaddstr(7, COLS - 35, msg);

	//スコアの表示
	attrset(COLOR_PAIR(4));
	sprintf(msg, "Score: %03d point", score);
    mvaddstr(8, COLS - 35, msg);

	//最大スコアの表示
	attrset(COLOR_PAIR(1));
	sprintf(msg, "HighScore: %03d point", highscore);
    mvaddstr(9, COLS - 35, msg);
    //経過時間をミリ秒で表示
    int end;
    end = clock();
	attrset(COLOR_PAIR(0));
    sprintf( msg,"started-time:%d[ms]", (end - start)/1000 );
    mvaddstr(10,COLS - 35, msg);
	refresh();
}

//ここから実行される
int main(int argc, char **argv){
    clock_t start, end;
    int i;
    int number1;
    int number2;
    int waitCount;
	//盤面サイズの入力
    printf("puyosize is 1_small, 2_alittlesmall, 3_normal\n");
    std::cout << "you write only 1,or,2,or,3>>" ;
    std::cin >> number1;
	//想定外の入力を除外
    if(number1>3 || number1<1){
        printf("it's wrong number\n");
        exit(0);
    }

	//ぷよの落下速度の入力
    printf("puyospeed is 1_slow, 2_normal, 3_fast, 4_veryfast\n");
    std::cout << "you write only 1,or,2,or,3,or,4>>" ;
    std::cin >> number2;
	//想定外の入力を除外
    if(number2>4 || number2<1){
        printf("it's wrong number\n");
        exit(0);
    }
	//時間計測開始
    start = clock();

	//インスタンスを宣言
	PuyoArrayActive puyo;
    PuyoArrayStack stack;
	PuyoControl ctrl;
	//画面の初期化
	initscr();
	//カラー属性を扱うための初期化
	start_color();

	//キーを押しても画面に表示しない
	noecho();
	//キー入力を即座に受け付ける
	cbreak();

	curs_set(0);
	//キー入力受付方法指定
	keypad(stdscr, TRUE);

	//キー入力非ブロッキングモード
	timeout(0);

    //連鎖数を保持
    int chain = 0;
	int chain1 = 0;
	int score = 0;
    int highscore = 0;
	//連続数を保持
	int drop = 0;
	int point = 0;

    int delay = 0;
	int puyostate = 0;
	int maxchain = 0;

    //初期化処理,盤面サイズの決定{
    if(number1==1){
        puyo.ChangeSize(LINES/4, COLS/4);
        }else if(number1==2){
        puyo.ChangeSize(LINES/3, COLS/3);	//フィールドは画面サイズの縦横1/3にする       
        }else if(number1==3){
        puyo.ChangeSize(LINES/2, COLS/2);
        }
    stack.ChangeSize(puyo.GetLine(), puyo.GetColumn());  //フィールドは画面サイズの縦横1/2にする
	ctrl.GeneratePuyo(puyo,stack);	//最初のぷよ生成

    if(number2==1){
        waitCount = 20000;
        }else if(number2==2){
        waitCount = 10000;	      
        }else if(number2==3){
        waitCount = 5000;
        }else if(number2==4){
        waitCount = 1000;
        }
	//メイン処理ループ
	while (1)
	{
		//キー入力受付
		int ch;
		ch = getch();
        //Qの入力で終了
		if (ch == 'Q')
	    {
          break;
	    }

		//入力キーごとの処理
		switch (ch)
		{
		case KEY_LEFT:
			ctrl.MoveLeft(puyo,stack);
			break;

		case KEY_RIGHT:
			ctrl.MoveRight(puyo,stack);
			break;

        case KEY_DOWN:
			//ぷよ下に移動
                ctrl.MoveDown(puyo,stack);
            break;
		case 'z':
			//ぷよ回転処理
			ctrl.Rotate(puyo,stack);
			break;

		default:
			break;
		}


		//処理速度調整のためのif文
		if (delay%waitCount == 0){
			//ぷよ下に移動
			ctrl.MoveDown(puyo,stack);
			Display(puyo, stack,chain,start,maxchain,score,highscore);
			//ぷよ着地判定
			if (ctrl.LandingPuyo(puyo,stack))
			{
				while(true) {
						//完全にぷよが落ち切るまで, 一段ずつ落とす処理を行う
                    	while (ctrl.DropPuyo(stack)) {
                        	//ぷよの様子を表示
                        	Display(puyo, stack,chain,start,maxchain,score,highscore);
							//時間を作る
                        	usleep(300000);
						}
						int a;
						a =ctrl.VanishPuyo(stack);
                    	if (a == 0) {
                        	//ぷよが一つも消えなければ終了
                        	break;
                    	}else {
							for(int b=0;b<a/4;b++){
                        	//ぷよが二種類以上消えて,ぷよが一種類消えるごとに, 連鎖数を1増やしてぷよを落とす処理を再度実行
                        	chain ++;
							chain1 ++;
							point ++;
							drop ++;
							}
                    	}
                	}
				//最大連鎖数の更新
				if(maxchain < chain1){
					maxchain = chain1;
				}
				chain1 = 0;
				//普通消し100点,連続消し＋150点
				if(drop>0){
					score += 100*point;
					score += 150*(drop-1);
				}else if(drop=0){
					score += 100*point;
				}
				//点数加算後,カウント初期化
				point = 0;
				drop = 0;
				//着地で新しいぷよ生成
				if(stack.GetValue(0, 5) == NONE && stack.GetValue(0, 6) == NONE ){
					ctrl.GeneratePuyo(puyo,stack);
				}else{
					char msg[256];
					attrset(COLOR_PAIR(1));
					sprintf(msg, "GAMEOVER,Quit is QQ,Restart is R");
    				mvaddstr(12, COLS - 35, msg);
                    //最大得点の更新
                    if(highscore < score){
                    highscore = score;
                    }
                    while(ctrl.GeneratePuyo(puyo,stack)){
                		//キー入力受付
                        int ch;
                        ch = getch();
                        //Rを押したら再スタート
                        if (ch == 'R')
                        {  
                            for (int y = 0; y < stack.GetLine(); y++) 
                            {
                                for (int x = 0; x < stack.GetColumn(); x++)  
                                {
                                    //盤面をリセット
                                    stack.SetValue(y, x,NONE); 
                                    ctrl.GeneratePuyo(puyo,stack);
                                    char msg[256];
                                    attrset(COLOR_PAIR(1));
                                    sprintf(msg, "Restart!!");
                                    mvaddstr(12, COLS - 35, msg);
                                    //最高スコア以外初期化
                                    chain = 0;
                                    chain1 = 0;
                                    score = 0;
                                    drop = 0;
                                    point = 0;
                                    maxchain =0;
                                    delay = 0;
                                    puyostate = 0;
									//時間計測開始
   									 start = clock();
                                } 
                            }
                        }
                        //Qの入力で終了
		                if (ch == 'Q')
		                {
			                break;
		                }     
                    }	  			
				}
			}	
		}
		delay++;

	    //表示
	    Display(puyo,stack,chain,start,maxchain,score,highscore);
	}

	//画面をリセット
	endwin();

	return 0;
}