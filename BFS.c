#pragma config(StandardModel, "EV3_REMBOT")

#define RED 1
#define GREEN 2
#define BLUE 4
#define YELLOW 8
#define WHITE 10

#define RIGHT 0
#define FWD 1
#define LEFT 2

#define boolean unsigned char
#define true 1
#define false 0

#define Nil -1
#define MaxEl 10

typedef struct
{
	int node;
	int turn;
	long degree;
} State;

typedef State infotype;
typedef int addr;   /* indeks tabel */

typedef struct
{
	infotype T[MaxEl+1];
	addr TOP;
} Stack;

#define Top(S) (S).TOP
#define InfoTop(S) (S).T[(S).TOP]

/* ************ Prototype ************ */
/* *** Konstruktor/Kreator *** */
void CreateEmpty (Stack *S)
{
	Top(*S)=Nil;
}

/* ************ Predikat Untuk test keadaan KOLEKSI ************ */
boolean IsEmpty (Stack S)
{
	return Top(S) == Nil;
}

boolean IsFull (Stack S)
{
	return Top(S) == MaxEl;
}

/* ************ Menambahkan sebuah elemen ke Stack ************ */
void Push (Stack *S, infotype X)
{
	Top(*S)++;
	InfoTop(*S).node = X.node;
	InfoTop(*S).turn = X.turn;
	InfoTop(*S).degree = X.degree;
}

/* ************ Menghapus sebuah elemen Stack ************ */
void Pop (Stack *S, infotype *X)
{
	*X = InfoTop(*S);
	Top(*S)--;
}

void createState(State *St, int n, int tr, long deg)
{
	(*St).node = n;
	(*St).turn = tr;
	(*St).degree = deg;
}


void LineTrack(int clr)
{
	int threshold = 60;
	int Kp = 10;
	int Tp = 45;
	int ki = 100;
	int integral = 0;

	int error = clr - threshold;
	integral = integral + error;
	int Turn = Kp * error + ki * integral;
	int powerA = Tp + Turn/100;
	int powerC = Tp - Turn/100;
	motor[leftMotor]  = powerC;
	motor[rightMotor] = powerA;
}

void goAhead(int n, int m)
{
	forward(n,milliseconds,m);
}

void goBack(int n, int m)
{
	backward(n, milliseconds, m);
}

int getColor(int R, int G, int B)
{
	if (R >= 90 && G <= 50 && B <= 50) return RED;
	if ((R <= 5 && G >= 90 && B <= 5) || (R < G && B < G && G > 50)) return GREEN;
	if ((R <= 15 && G <= 15 && B >= 90) || (R + G <= B + 15 && B >= 95)) return BLUE;
	if (R >= 90 && G >= 90 && B <= 50) return YELLOW;
	return 0;
}

void turn(int dir, int deg, int m, int n)
{
	int currDeg = getGyroDegrees(gyroSensor);

	if (dir == LEFT)
	{
		setMotorSpeed(leftMotor,n);
		setMotorSpeed(rightMotor,m);
	}

	else if (dir == RIGHT)
	{
		setMotorSpeed(leftMotor,m);
		setMotorSpeed(rightMotor,n);
	}

	while (getMotorMoving(leftMotor) || getMotorMoving(rightMotor))
	{
		if ((getGyroDegrees(gyroSensor)-currDeg>deg && dir==RIGHT) || (getGyroDegrees(gyroSensor)-currDeg<deg && dir==LEFT))
		{
			break;
		}
	}
}

void cek_simpang (Stack *S, int currDeg, int node, int count)
{
	// ambil kanan atau depan
	turn(RIGHT, 90, 45, 0);
	goAhead(150,30);
	if (getColorReflected(colorSensor)>10)
	{
		goBack(600,50);
		turn(LEFT, -90, 45, 0);
		goAhead(150,30);
		count = count+1;
	}

	State St;
	createState(&St,node,count,currDeg);

	Push(S,St);
}

void Bersih()
{
	displayTextLine(1,"");
	displayTextLine(2,"");
	displayTextLine(3,"");
	displayTextLine(4,"");
	displayTextLine(5,"");
	displayTextLine(6,"");
}

void update_simpang (Stack *S, int node)
{
	// ambil kanan atau depan
	turn(RIGHT, 90, 55, 0);
	goAhead(200,30);
	int turn2 = InfoTop(*S).turn+1;
	if (getColorReflected(colorSensor)>10)
	{
		goBack(600,50);
		turn(LEFT, -90, 55, 0);
		goAhead(200,30);
		turn2++;
	}

	State St, St2;
	Pop(S,&St);
	//displayTextLine(1, "TURN %d", junk);
	if(turn2<3){
		createState(&St2,node,turn2,St.degree);
		Push(S,St2);
		Bersih();
		displayTextLine(2, "UPDATE ATRIBUT NODE BARU!");
		displayTextLine(3, "%d %d", InfoTop(*S).node, InfoTop(*S).turn);
	}
}

int IDS(int deep)
{
	int color; int R,G,B; int node=0;
	int junk = 0;
	resetGyro(gyroSensor);
	Stack S; CreateEmpty(&S);
	State St; createState(&St,0,0,0);
	Push(&S,St); // dummy
	int found = 0;
	int bfsRun = 1;

	while(!IsEmpty(S))
	{
			bfsRun = node<=deep || found;
			Bersih();
			//displayTextLine(1, "BFS %d ", bfsRun);
  		int currDeg;
  		color = getColorReflected(colorSensor);
  		getColorRGB(colorSensor,R,G,B);
  		LineTrack(color);
  		if(getColor(R,G,B))
  		{
			if (getColor(R,G,B) == GREEN && bfsRun)
			{
				if(!found)
				{
					node++;
					bfsRun = node<=deep || found;
					if(!bfsRun) {
						turn(LEFT,-180,30,0);
						goAhead(100,30);
						continue;
					}
					goAhead(500,50);
					currDeg = getGyroDegrees(gyroSensor);
					Bersih();
					displayTextLine(1, "%d = %d", node, InfoTop(S).node);
					if (InfoTop(S).node != node)
					{
						//junk = 0;
						cek_simpang(&S, currDeg, node, 0);
						Bersih();
						displayTextLine(1, "Atribut dari node %d adalah: ", node);
						displayTextLine(2, "%d %d", InfoTop(S).node, InfoTop(S).turn);
					}
					else
					{
						//junk += 1;
						int t = InfoTop(S).turn+1;
						update_simpang(&S, node);
						if(t>=3) {
							displayTextLine(1, "Tdk ada jalur node %d", node);
							displayTextLine(2, "Hapus");
							node--;
							node--;
						}
						//else InfoTop(S).turn = junk;
					}
				}
				else
				{
					int count = 0;
					infotype temp;
					Bersih();
					displayTextLine(1, "Atribut dari node %d adalah: ", node);
					displayTextLine(2, "well %d %d", InfoTop(S).node, InfoTop(S).turn);
					displayTextLine(3, "Pop out!");
					goAhead(300,60);
					if (InfoTop(S).turn == LEFT)
					{
						turn(RIGHT,90,45,0);
					}
					else if (InfoTop(S).turn == RIGHT)
					{
						turn(LEFT,-90,45,20);
					} else {
						goAhead(400,200);
					}
					Pop(&S, &temp);
				}
			}
  		else if(getColor(R,G,B) == GREEN && !bfsRun){
					int count = 0;
					infotype temp;
					Bersih();
					displayTextLine(1, "Atribut dari node %d adalah: ", node);
					displayTextLine(2, "well %d %d", InfoTop(S).node, InfoTop(S).turn);
					displayTextLine(3, "Pop out!");
					goAhead(300,60);
					if (InfoTop(S).turn == LEFT)
					{
						//displayTextLine(4, "MASUK");
						turn(RIGHT,90,45,0);
					}
					else if (InfoTop(S).turn == RIGHT)
					{
						turn(LEFT,-90,45,20);
					} else {
						goAhead(400,200);
					}
					Pop(&S, &temp);
  		}
			else if (getColor(R,G,B) == RED || (getColor(R,G,B) == GREEN && node>deep))
			{
				int coltemp = getColor(R,G,B);
				turn(LEFT,-180,30,0);
				goAhead(100,30);
				if(coltemp == RED) node--;
				clearDebugStream();
				displayTextLine(1, "FOUND RED, delete %d node", node+1);
			}

			else if (getColor(R,G,B) == YELLOW)
			{
				found = 1;
				Bersih();
				displayTextLine(1,"**************************");
				displayTextLine(2,"********** API ***********");
				displayTextLine(3,"******** BERHASIL ********");
				displayTextLine(4,"******* DIPADAMKAN *******");
				displayTextLine(5,"**************************");
				displayTextLine(6,"**************************");
			}
			else if (getColor(R,G,B) == BLUE)
			{
				infotype temps;
				Bersih();
				displayTextLine(1,"Alhamdulillah tiba dengan selamat");
				goBack(300,30);
				turn(LEFT,-180,45,0);
				stopAllMotors();
				while(!IsEmpty(S)){
					Pop(&S, &temps);
				}
				motor[leftMotor]  = 0;
				motor[rightMotor] = 0;
				break;
			}
  	}
	}
	return found;
}

task main()
{
	int R, G, B;
	getColorRGB(colorSensor,R,G,B);
	while(getColor(R,G,B)!=BLUE)
	{
		motor[leftMotor]  = 50;
    	motor[rightMotor] = 50;
    	getColorRGB(colorSensor,R,G,B);
	}
	turn(LEFT,15,15,100);
	goAhead(400,200);
	int i =1;
	while(true){
		if(IDS(i)) break;
		i++;
	}
}
