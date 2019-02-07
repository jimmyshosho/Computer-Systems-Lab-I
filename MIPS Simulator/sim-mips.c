// List the full names of ALL group members at the top of your code.
// DHIMITER SHOSHO
// ANDREW JEWERS
// SAMUEL FICK
// ALEX DONADIO
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
//feel free to add here any additional library names you may need
#define SINGLE 1
#define BATCH 0
#define REG_NUM 32

// VARIABLES & STRUCTS DEFINED HERE//
enum opcodes {ADD=0,SUB=0,MUL=0,ADDI=8,BEQ=4,LW=35,SW=43}; //enumerated types for opcode
enum funct {ADDF=32,SUBF=34,MULF=24};
struct inst // struct used for each instruction memory
{
    int opcode;
    int rs;
    int rt;
    int rd;
    int funct;
    int immediate;
};
struct latch //holds the contents of registers in between pipeline stages
{
    struct inst instruction;
    int rsContents;
    int rtContents;
    int regResult;
    int numCycles;
};

/*struct for each register that holds the contents
and a variable to check the validity of the register*/
struct reg
{
    int contents;
    int flag;
};

int branch = 1;// flag used to check if we are branching or not
int PC = 0; //program counter for instruction memory
struct reg registers[REG_NUM]; //holds the values for all the registers
struct inst instructionMemory[512]; // array used to store instruction memory. each entry holds 4 bytes
int dataMemory[512]; // array to store the data memory in hex
static const char* registerNames[64] = {"zero","at","v0","v1","a0","a1","a2","a3",
                                        "t0","t1","t2","t3","t4","t5","t6","t7",
                                        "s0","s1","s2","s3","s4","s5","s6","s7",
                                        "t8","t9","k0","k1","gp","sp","fp","ra",
                                        "0", "1", "2", "3", "4", "5", "6", "7", "8",
                                        "9","10","11","12","13","14","15","16","17",
                                        "18","19","20","21","22","23","24","25","26",
                                        "27","28","29","30","31"};
//define your own counter for the usage of each pipeline stage here
int ifCounter = 0;
int idCounter = 0;
int exCounter = 0;
int memCounter = 0;
int wbCounter = 0;
// VARIABLES END HERE

//FUNCTIONS DEFINED HERE//
char* progScanner(char command[]);// used to take input from a file, adjusts it as needed, and checks for syntax errors
char* regNumberConverter(char* commandv2);//Converts register names to numbers from 0-31
struct inst parser(char* commandv3);// takes regNumberConverter output and parses into its parts, returning an inst struct
int IF(struct latch*, int);
int ID(struct latch*, struct latch*);
int EX(struct latch*, struct latch*, int, int);
int MEM(struct latch*, struct latch*, int);
int WB(struct latch*);
// FUNCTIONS END HERE

//HELPER FUNCTIONS DEFINED HERE//
void dollarsignChecker(char* cmd,int numSigns);//checks for valid number of dollar signs for each instruction
int isValid(char* reg); // checks if register is valid
int createMask(int a, int b);// a=startbit b=endbit    returns mask
int STRCMP (const char *p1, const char *p2); // string comparator function
char* STRCAT(char* destination, const char* source); // string concatenator function
char* STRSTR(char *str, char *substr);// checks to see if string exists
//HELPER FUNCTIONS END HERE

int main (int argc, char *argv[])
{
    int sim_mode=0;//mode flag, 1 for single-cycle, 0 for batch
    int c,m,n;
    int i;//for loop counter
    long pgm_c=0;//program counter
    long sim_cycle=0;//simulation cycle counter
    //define your own counter for the usage of each pipeline stage here

    int test_counter=0;
    FILE *input=NULL;
    FILE *output=NULL;
    printf("The arguments are:");
    for(i=1;i<argc;i++)
        printf("%s ",argv[i]);
    printf("\n");

    if(argc==7)
    {
        if(STRCMP ("-s",argv[1])==0)
            sim_mode=SINGLE;
        else if(STRCMP("-b",argv[1])==0)
            sim_mode=BATCH;
        else
        {
            printf("Wrong sim mode chosen\n");
            exit(0);
        }
        m=atoi(argv[2]);
        n=atoi(argv[3]);
        c=atoi(argv[4]);
        input=fopen(argv[5],"r");
        output=fopen(argv[6],"w");
    }
    else
    {
        printf("Usage: ./sim-mips -s m n c input_name output_name (single-sysle mode)\n or \n ./sim-mips -b m n c input_name  output_name(batch mode)\n");
        printf("m,n,c stand for number of cycles needed by multiplication, other operation, and memory access, respectively\n");
        exit(0);
    }
    if(input==NULL)
    {
        printf("Unable to open input or output file\n");
        exit(0);
    }
    if(output==NULL)
    {
        printf("Cannot create output file\n");
        exit(0);
    }

    //Latches for in between pipeline stages
    struct latch* IF_ID = (struct latch*) malloc(sizeof(struct latch)); // gives space to latches
    struct latch* ID_EX = (struct latch*) malloc(sizeof(struct latch)); // gives space to latches
    struct latch* EX_MEM = (struct latch*) malloc(sizeof(struct latch)); // gives space to latches
    struct latch* MEM_WB = (struct latch*) malloc(sizeof(struct latch)); // gives space to latches

    char command[100]; // array to hold the commands in file
    int count = 0;// mallocs memory for instructionMemory used to store MIP instructions
    while( fgets(command,100,input) != NULL) // while the file contains more commands, keep running
    {
        instructionMemory[count]= parser(regNumberConverter(progScanner(command)));
        count++;
    }
    for(i=0; i<32;i++) // initially all the registers should be safe as they are unused
      registers[i].flag = 1;

    while(1) // keep running until haltSimulation is seen
      {
        if(sim_mode==1)
        {
        		printf("cycle: %ld register value: ",sim_cycle);
        		for (i=1;i<REG_NUM;i++)
        			printf("%d  ",registers[i].contents);

        		printf("program counter: %d\n",PC);
        		printf("press ENTER to continue\n");
        		while(getchar() != '\n');
        	}
        	sim_cycle+=1;
        if(WB(MEM_WB) != 0)
        {
          if(MEM(EX_MEM, MEM_WB,c) != 0)
            if(EX(ID_EX,EX_MEM,m,n) != 0)
                if(ID(IF_ID,ID_EX) != 0)
                   if(IF(IF_ID,c) != 0)
                        PC= PC + 1;
        }
        else
          break;
      }
    if(sim_mode==0)
    {
      double ifUtil = (double) ifCounter / sim_cycle;
      double idUtil = (double) idCounter / sim_cycle;
      double exUtil = (double) exCounter / sim_cycle;
      double memUtil = (double) memCounter / sim_cycle;
      double wbUtil = (double) wbCounter / sim_cycle;
 		  fprintf(output,"program name: %s\n",argv[5]);
 		  fprintf(output,"stage utilization: %f  %f  %f  %f  %f \n",ifUtil, idUtil, exUtil, memUtil, wbUtil);
 		  fprintf(output,"register values ");
 		  for (i=1;i<REG_NUM;i++)
 			  fprintf(output,"%d  ",registers[i].contents);
 		  fprintf(output,"%d\n",PC);
   	}

 	//close input and output files at the end of the simulation
 	fclose(input);
 	fclose(output);
 	return 0;

}

char* progScanner(char command[])// used to take input from a file, adjusts it as needed, and checks for syntax errors
{
    char* newCommand = (char*) malloc(100*sizeof(char));

    if(command[0]=='b' || command[0]=='m' || command[0]=='a' || (command[0]=='s' && command[1]=='u'))
    {// used for beq, mul, add, addi, and sub

        char delimiters[]= " ,();\n";
        char* pieces[4];
        pieces[0] = strtok(command, delimiters);
        pieces[1] = strtok(NULL, delimiters);
        pieces[2] = strtok(NULL, delimiters);
        pieces[3] = strtok(NULL, delimiters);

        if(pieces[3] == NULL)
        {
          int i;
          printf("\n***ERROR: INVALID COMMAND OR INVALID INSTRUCTION DETECTED***\n");
          printf("COMMAND: ");
          for(i = 0; i<4; i++)
            printf("%s ", pieces[i]);
            printf(" <--\n");
          exit(0);
        }

        STRCAT(newCommand, pieces[0]);
        STRCAT(newCommand, " ");
        STRCAT(newCommand, pieces[1]);
        STRCAT(newCommand, " ");
        STRCAT(newCommand, pieces[2]);
        STRCAT(newCommand, " ");
        STRCAT(newCommand, pieces[3]);

        return newCommand;
    }
    if(command[0]=='l' || (command[0]=='s' && command[1]=='w'))// used for lw and sw
    {
        int i;
        int openPCount = 0;
        int closedPCount = 0;
        for(i = 0; i<100; i++)
        {
            if(command[i] == '(')
                openPCount++;
            if(command[i] == ')')
                closedPCount++;
            if(command[i] == '\0')
                break;
        } // loop checks for parenthesis mismatch...

        if(openPCount != closedPCount)
        {
            printf("\n***ERROR: EXTRA OR MISMATCHED PARENTHESIS DETECTED***\n");
            printf("COMMAND: %s\n", command);
            exit(0);
        }
        else
        {
            char delimiters[]= " ,();\n";
            char* pieces[4];
            pieces[0] = strtok(command, delimiters);
            pieces[1] = strtok(NULL, delimiters);
            pieces[2] = strtok(NULL, delimiters);
            pieces[3] = strtok(NULL, delimiters);

            if(pieces[3] == NULL)// edge case that checks incase sw or lw doesnt have an exlicit offset
            {
                pieces[3] = pieces[2];
                pieces[2] = "0";
            }

            STRCAT(newCommand, pieces[0]);
            STRCAT(newCommand, " ");
            STRCAT(newCommand, pieces[1]);
            STRCAT(newCommand, " ");
            STRCAT(newCommand, pieces[2]);
            STRCAT(newCommand, " ");
            STRCAT(newCommand, pieces[3]);

            return newCommand;
        }
    }
    if(command[0]=='h') // used for haltSimulation
    {
        char delimiters[]= " ,();\n";
        char* pieces[1];
        pieces[0] = strtok(command, delimiters);
        STRCAT(newCommand, pieces[0]);

        return newCommand;
    }
    if(command[0]!='l' && command[0]!='s' && command[0]!='m' && command[0]!='h' && command[0]!='a' && command[0]!='b')
    {
        printf("\n***ERROR: COMMAND DOES NOT EXIST OR SYNTAX ERROR***\n");
        printf("COMMAND: %s\n", command);
        exit(0);
    }

    return NULL;
}

char* regNumberConverter(char* commandv2)
{
    if(STRCMP(commandv2,"haltSimulation")!=0)
    {
        int index = 0; //index for fsm for $ checker
        int numSigns = 0;

        //Code here is fsm for checking the number of dollar signs based on opcodes
        //Errors in opcodes are not caught here, they are caught in parser
        if(commandv2[index]=='a')//Check for add or addi
        {
            index++;
            if(commandv2[index]=='d')
            {
                index++;
                if(commandv2[index]=='d')
                {
                    index++;
                    if(commandv2[index]==' ')
                    {
                        numSigns = 3; //check for three dollar signs
                        dollarsignChecker(commandv2,numSigns);
                    }
                    else if(commandv2[index]=='i')
                    {
                        numSigns = 2; //check for two dollar signs
                        dollarsignChecker(commandv2,numSigns);
                    }
                }
            }
        }
        if(commandv2[index]=='s')//Check for sub or sw
        {
            index++;
            if(commandv2[index]=='u')
            {
                numSigns = 3;//check for three dollar signs
                dollarsignChecker(commandv2,numSigns);
            }
            else if(commandv2[index]=='w')
            {
                numSigns = 2; //check for two dollar signs
                dollarsignChecker(commandv2,numSigns);
            }
        }
        if(commandv2[index]=='m')//Check for mul
        {
            numSigns = 3; //check for three dollar signs
            dollarsignChecker(commandv2,numSigns);
        }
        if(commandv2[index]=='b')//Check for beq
        {
            numSigns = 2; //check for two dollar signs
            dollarsignChecker(commandv2,numSigns);
        }
        if(commandv2[index]=='l')//Check for lw
        {
            numSigns = 2; //check for two dollar signs
            dollarsignChecker(commandv2,numSigns);
        }

        int j;
        char delims[] = " "; // want to delim by spaces
        char* temp[4];
        temp[0] = strtok(commandv2, delims);
        temp[1] = strtok(NULL, delims);
        temp[2] = strtok(NULL, delims);
        temp[3] = strtok(NULL, delims);

        // going to now check to make sure registers are valid
        for(j=1;j<4;j++)
        {
            if(isValid(temp[j])==0)
            {
                printf("\n***ERROR: INVALID REGISTER OR REGISTER OUT OF BOUNDS\n");
                printf("COMMAND: %s %s %s %s\n", temp[0],temp[1],temp[2],temp[3]);
            }
        }
        // we need to now reset commandv2...
        char* oldCommand = (char*) malloc(100*sizeof(char));
        STRCAT(oldCommand, temp[0]);
        STRCAT(oldCommand, " ");
        STRCAT(oldCommand, temp[1]);
        STRCAT(oldCommand, " ");
        STRCAT(oldCommand, temp[2]);
        STRCAT(oldCommand, " ");
        STRCAT(oldCommand, temp[3]);
        commandv2=oldCommand;// gives commandv2 its original information

        int i; //for loop variable
        int k; //for loop variable
        char delimiters[] = " $\n";//Remove spaces and dollar signs and store only register names
        char* fields[4];//holds the mips instruction fields
        //Splitting the instruction into individual strings
        fields[0] = strtok(commandv2, delimiters);
        fields[1] = strtok(NULL, delimiters);
        fields[2] = strtok(NULL, delimiters);
        fields[3] = strtok(NULL, delimiters);

        for(i=1;i<=3;i++) //Start at first register and go through all three registers
        {
            if(STRCMP(fields[i], "zero")== 0 || STRCMP(fields[i], "0") == 0)
                fields[i]="0";
            else if(STRCMP(fields[i], "at") == 0 || STRCMP(fields[i], "1") == 0)
                fields[i]="1";
            else if(STRCMP(fields[i], "v0") == 0 || STRCMP(fields[i], "2") == 0)
                fields[i]="2";
            else if(STRCMP(fields[i], "v1") == 0 || STRCMP(fields[i], "3") == 0)
                fields[i]="3";
            else if(STRCMP(fields[i], "a0") == 0 || STRCMP(fields[i], "4") == 0)
                fields[i]="4";
            else if(STRCMP(fields[i], "a1") == 0 || STRCMP(fields[i], "5") == 0)
                fields[i]="5";
            else if(STRCMP(fields[i], "a2") == 0 || STRCMP(fields[i], "6") == 0)
                fields[i]="6";
            else if(STRCMP(fields[i], "a3") == 0 || STRCMP(fields[i], "7") == 0)
                fields[i]="7";
            else if(STRCMP(fields[i], "t0") == 0 || STRCMP(fields[i], "8") == 0)
                fields[i]="8";
            else if(STRCMP(fields[i], "t1") == 0 || STRCMP(fields[i], "9") == 0)
                fields[i]="9";
            else if(STRCMP(fields[i], "t2") == 0 || STRCMP(fields[i], "10") == 0)
                fields[i]="10";
            else if(STRCMP(fields[i], "t3") == 0 || STRCMP(fields[i], "11") == 0)
                fields[i]="11";
            else if(STRCMP(fields[i], "t4") == 0 || STRCMP(fields[i], "12") == 0)
                fields[i]="12";
            else if(STRCMP(fields[i], "t5") == 0 || STRCMP(fields[i], "13") == 0)
                fields[i]="13";
            else if(STRCMP(fields[i], "t6") == 0 || STRCMP(fields[i], "14") == 0)
                fields[i]="14";
            else if(STRCMP(fields[i], "t7") == 0 || STRCMP(fields[i], "15") == 0)
                fields[i]="15";
            else if(STRCMP(fields[i], "s0") == 0 || STRCMP(fields[i], "16") == 0)
                fields[i]="16";
            else if(STRCMP(fields[i], "s1") == 0 || STRCMP(fields[i], "17") == 0)
                fields[i]="17";
            else if(STRCMP(fields[i], "s2") == 0 || STRCMP(fields[i], "18") == 0)
                fields[i]="18";
            else if(STRCMP(fields[i], "s3") == 0 || STRCMP(fields[i], "19") == 0)
                fields[i]="19";
            else if(STRCMP(fields[i], "s4") == 0 || STRCMP(fields[i], "20") == 0)
                fields[i]="20";
            else if(STRCMP(fields[i], "s5") == 0 || STRCMP(fields[i], "21") == 0)
                fields[i]="21";
            else if(STRCMP(fields[i], "s6") == 0 || STRCMP(fields[i], "22") == 0)
                fields[i]="22";
            else if(STRCMP(fields[i], "s7") == 0 || STRCMP(fields[i], "23") == 0)
                fields[i]="23";
            else if(STRCMP(fields[i], "t8") == 0 || STRCMP(fields[i], "24") == 0)
                fields[i]="24";
            else if(STRCMP(fields[i], "t9") == 0 || STRCMP(fields[i], "25") == 0)
                fields[i]="25";
            else if(STRCMP(fields[i], "k0") == 0 || STRCMP(fields[i], "26") == 0)
                fields[i]="26";
            else if(STRCMP(fields[i], "k1") == 0 || STRCMP(fields[i], "27") == 0)
                fields[i]="27";
            else if(STRCMP(fields[i], "gp") == 0 || STRCMP(fields[i], "28") == 0)
                fields[i]="28";
            else if(STRCMP(fields[i], "sp") == 0 || STRCMP(fields[i], "29") == 0)
                fields[i]="29";
            else if(STRCMP(fields[i], "fp") == 0 || STRCMP(fields[i], "30") == 0)
                fields[i]="30";
            else if(STRCMP(fields[i], "ra") == 0 || STRCMP(fields[i], "31") == 0)
                fields[i]="31";
        }

        //Putting the revised instruction back into one string
        char* commandv3 = (char*) malloc(100*sizeof(char));
        STRCAT(commandv3, fields[0]);
        STRCAT(commandv3, " ");
        STRCAT(commandv3, fields[1]);
        STRCAT(commandv3, " ");
        STRCAT(commandv3, fields[2]);
        STRCAT(commandv3, " ");
        STRCAT(commandv3, fields[3]);
        return commandv3;
    }
    else
        return commandv2;

}

struct inst parser(char* commandv3)
{
    struct inst instruction;
    instruction.opcode = -1;
    instruction.rs = -1;
    instruction.rt = -1;
    instruction.rd = -1;
    instruction.immediate = -1;
    instruction.funct = -1;

    char delimiters[] = " \n"; // uses spaces as delimiter
    char* fields[4];//holds the mips instruction fields

    //Splitting the instruction into individual strings
    fields[0] = strtok(commandv3, delimiters);
    fields[1] = strtok(NULL, delimiters);
    fields[2] = strtok(NULL, delimiters);
    fields[3] = strtok(NULL, delimiters);

    //if add sub or mul, set opcode to respective enumerated opcode, then first instruction in address is RD, 2nd is RS, 3rd is RT. Immediate stays -1.
    //if lw, set opcode to respective enumerated opcode, then first instruction in address is RT, 2nd is immediate, 3rd is RS.
    //if sw, set opcode to respective enmuerated opcode, then first instruction in address is RS, 2nd is immediate, 3rd is RT.
    //if beq, set opcode to respective enumerated opcode,then first instruction in address is RS, 2nd is RT, 3rd is immediate.
    //if addi, set opcode to respective enumerated opcode, then first instruction in address is RT, 2nd is RS, 3rd is immediate.
    if(STRCMP(fields[0],"add")==0)
    {
        instruction.opcode = ADD;
        instruction.rs = atoi(fields[2]);
        instruction.rt = atoi(fields[3]);
        instruction.rd = atoi(fields[1]);
        instruction.funct = 32;
    }

    else if(STRCMP(fields[0],"sub")==0)
    {
        instruction.opcode = SUB;
        instruction.rs = atoi(fields[2]);
        instruction.rt = atoi(fields[3]);
        instruction.rd = atoi(fields[1]);
        instruction.funct = 34;
    }

    else if(STRCMP(fields[0],"mul")==0)
    {
        instruction.opcode = MUL;
        instruction.rs = atoi(fields[2]);
        instruction.rt = atoi(fields[3]);
        instruction.rd = atoi(fields[1]);
        instruction.funct = 24;
    }

    else if(STRCMP(fields[0],"lw")==0)
    {
        instruction.opcode = LW;
        instruction.rs = atoi(fields[3]);
        instruction.rt = atoi(fields[1]);
        instruction.immediate = atoi(fields[2]);
        if(instruction.immediate >= 65536)
        {
            printf("\n***ERROR: IMMEDIATE FIELD CONTAINS NUMBER TO LARGE TO STORE IN 16 BITS***\n");
            printf("COMMAND: %s %s %s %s\n", fields[0],fields[1],fields[2],fields[3]);
            exit(0);
        }
        if((instruction.immediate%4)!=0)
        {
            printf("\n*** ERROR: OFFSET OF LW COMMAND IS NOT A FACTOR OF 4***\n");
            printf("COMMAND: %s %s %s %s\n", fields[0],fields[1],fields[2],fields[3]);
            exit(0);
        }

    }
    else if(STRCMP(fields[0],"sw")==0)
    {
        instruction.opcode = SW;
        instruction.rs = atoi(fields[3]);
        instruction.rt = atoi(fields[1]);
        instruction.immediate = atoi(fields[2]);
        if(instruction.immediate >= 65536)
        {
            printf("\n***ERROR: IMMEDIATE FIELD CONTAINS NUMBER TO LARGE TO STORE IN 16 BITS***\n");
            printf("COMMAND: %s %s %s %s\n", fields[0],fields[1],fields[2],fields[3]);
            exit(0);
        }
        if((instruction.immediate%4)!=0)
        {
            printf("\n*** ERROR: OFFSET OF SW COMMAND IS NOT A FACTOR OF 4***\n");
            printf("COMMAND: %s %s %s %s\n", fields[0],fields[1],fields[2],fields[3]);
            exit(0);
        }
    }
    else if(STRCMP(fields[0],"addi")==0)
    {
        instruction.opcode = ADDI;
        instruction.rs = atoi(fields[2]);
        instruction.rt = atoi(fields[1]);
        instruction.immediate = atoi(fields[3]);
        if(instruction.immediate >= 65536)
        {
            printf("\n***ERROR: IMMEDIATE FIELD CONTAINS NUMBER TO LARGE TO STORE IN 16 BITS***\n");
            printf("COMMAND: %s %s %s %s\n", fields[0],fields[1],fields[2],fields[3]);
            exit(0);
        }
    }
    else if(STRCMP(fields[0],"beq")==0)
    {
        instruction.opcode = BEQ;
        instruction.rs = atoi(fields[1]);
        instruction.rt = atoi(fields[2]);
        instruction.immediate = atoi(fields[3]);
        if(instruction.immediate >= 65536)
        {
            printf("\n***ERROR: IMMEDIATE FIELD CONTAINS NUMBER TO LARGE TO STORE IN 16 BITS***\n");
            printf("COMMAND: %s %s %s %s\n", fields[0],fields[1],fields[2],fields[3]);
            exit(0);
        }
    }
    else if(STRCMP(fields[0],"haltSimulation")==0){/*do nothing as we want to keep values set to -1*/}
    else
    {
        printf("\n***INVALID OPCODE OR MISPELLED COMMAND ERROR***\n");
        printf("COMMAND: %s %s %s %s\n", fields[0],fields[1],fields[2],fields[3]);
        exit(0);
    }
    return instruction;
}

int isValid(char* reg)
{
    int i;
    if(STRSTR(reg,"$") != NULL)
    {
        char delimiters[] = " $\n"; // uses $ as delimiter
        char* fields[1];//holds the mips instruction fields
        //Splitting the instruction into individual strings
        fields[0] = strtok(reg, delimiters);
        for(i=0; i<64; i++)
        {
            if(STRCMP(fields[0], registerNames[i]) == 0)
                return 1;
        }
        return 0;
    }
    return 1;
}

void dollarsignChecker(char* cmd,int numSigns) //helper method for regNumberConverter
{
    int i; //counter used in for loops
    int length = strlen(cmd); //length of the input string
    int actualNum = 0; //actual number of dollar signs found
    if(numSigns==3)
    {
        for(i=0;i<length;i++)
        {
            if(cmd[i]=='$')
                actualNum++;
        }
        if(actualNum!=3)
        {
            printf("Syntax Error: Invalid number of dollar signs in instruction:\n");
            printf("%s\n",cmd);
            exit(0);
        }

    }
    else if(numSigns==2)
    {
        for(i=0;i<length;i++)
        {
            if(cmd[i]=='$')
                actualNum++;
        }
        if(actualNum!=2)
        {
            printf("Syntax Error: Invalid number of dollar signs in instruction:\n");
            printf("%s\n",cmd);
            exit(0);
        }
    }
}

int IF(struct latch* IF_ID, int cycles)
{
    //"fetching" the instruction from the array
    struct inst currInstruction = instructionMemory[PC];
    if(currInstruction.opcode == -1) //haltSimulation detected, exit function
    {
        IF_ID->instruction = currInstruction;
        return 0;
    }
    ifCounter++;

    if(IF_ID->numCycles == 0)
        IF_ID->numCycles = cycles; //set numCycles for latch

    if(branch == 0)//nop until branch is determined
    {
      IF_ID->instruction.opcode = ADD;
      IF_ID->instruction.rs = 0;
      IF_ID->instruction.rt = 0;
      IF_ID->instruction.rd = 0;
      IF_ID->instruction.funct = ADDF;
      IF_ID->instruction.immediate = 0;
      IF_ID->rsContents = 0;
      IF_ID->rtContents = 0;
      IF_ID->regResult = 0;
      return 0;
    }

    IF_ID->numCycles--;
    if (IF_ID->numCycles == 0)
    {
        IF_ID->instruction = currInstruction;
        return 1;
    }
}

int ID(struct latch* IF_ID, struct latch* ID_EX)
{
    if (IF_ID->instruction.opcode == -1) // checks to see if its haltSimulation
    {
        ID_EX->instruction.opcode = -1;
        ID_EX->instruction.rs = -1;
        ID_EX->instruction.rt = -1;
        ID_EX->instruction.rd = -1;
        ID_EX->instruction.immediate = -1;
        ID_EX->rsContents = -1;
        ID_EX->rtContents = -1;
        ID_EX->regResult = -1;
        return 0;
    }
    if(branch == 0)//nop until branch is determined
    {
      ID_EX->instruction.opcode = ADD;
      ID_EX->instruction.rs = 0;
      ID_EX->instruction.rt = 0;
      ID_EX->instruction.rd = 0;
      ID_EX->instruction.funct = ADDF;
      ID_EX->instruction.immediate = 0;
      ID_EX->rsContents = 0;
      ID_EX->rtContents = 0;
      ID_EX->regResult = 0;
      return 0;
    }
    idCounter++;
    // Check that registers are unflagged
    // flag == false, register not safe
    // go in here if mul, add or sub command
    if(IF_ID->instruction.opcode==ADD && IF_ID->instruction.funct==ADDF)
    {
      if(registers[IF_ID->instruction.rs].flag && registers[IF_ID->instruction.rt].flag)
      {
        ID_EX->instruction = IF_ID->instruction;// UPDATES INSTRUCTION TO OTHER LATCH
        ID_EX->rsContents = registers[IF_ID->instruction.rs].contents; //pass reg contents to variable
        ID_EX->rtContents = registers[IF_ID->instruction.rt].contents; //pass reg contents to variable
        ID_EX->regResult = 0; // clears old regResult
        registers[ID_EX->instruction.rd].flag = 0; // NOT SAFE TO USE ANYMORE
        return 1;
      }
      else
      {//send NOP add $0, $s0, $s0
         ID_EX->instruction.opcode = ADD;
         ID_EX->instruction.rs = 0;
         ID_EX->instruction.rt = 0;
         ID_EX->instruction.rd = 0;
         ID_EX->instruction.funct = ADDF;
         ID_EX->instruction.immediate = 0;
         ID_EX->rsContents = 0;
         ID_EX->rtContents = 0;
         ID_EX->regResult = 0;
         return 0;
      }
    }

    if(IF_ID->instruction.opcode==SUB && IF_ID->instruction.funct==SUBF)
    {
      if(registers[IF_ID->instruction.rs].flag && registers[IF_ID->instruction.rt].flag)
      {
        ID_EX->instruction = IF_ID->instruction;// UPDATES INSTRUCTION TO OTHER LATCH
        ID_EX->rsContents = registers[IF_ID->instruction.rs].contents; //pass reg contents to variable
        ID_EX->rtContents = registers[IF_ID->instruction.rt].contents; //pass reg contents to variable
        ID_EX->regResult = 0; // clears old regResult
        registers[ID_EX->instruction.rd].flag = 0; // NOT SAFE TO USE ANYMORE
        return 1;
      }
      else
      {//send NOP add $0, $s0, $s0
         ID_EX->instruction.opcode = ADD;
         ID_EX->instruction.rs = 0;
         ID_EX->instruction.rt = 0;
         ID_EX->instruction.rd = 0;
         ID_EX->instruction.funct = ADDF;
         ID_EX->instruction.immediate = 0;
         ID_EX->rsContents = 0;
         ID_EX->rtContents = 0;
         ID_EX->regResult = 0;
         return 0;
      }
    }

    if(IF_ID->instruction.opcode==MUL && IF_ID->instruction.funct==MULF)
    {
      if(registers[IF_ID->instruction.rs].flag && registers[IF_ID->instruction.rt].flag)
      {
        ID_EX->instruction = IF_ID->instruction;// UPDATES INSTRUCTION TO OTHER LATCH
        ID_EX->rsContents = registers[IF_ID->instruction.rs].contents; //pass reg contents to variable
        ID_EX->rtContents = registers[IF_ID->instruction.rt].contents; //pass reg contents to variable
        ID_EX->regResult = 0; // clears old regResult
        registers[ID_EX->instruction.rd].flag = 0; // NOT SAFE TO USE ANYMORE
        return 1;
      }
      else
      {//send NOP add $0, $s0, $s0
         ID_EX->instruction.opcode = ADD;
         ID_EX->instruction.rs = 0;
         ID_EX->instruction.rt = 0;
         ID_EX->instruction.rd = 0;
         ID_EX->instruction.funct = ADDF;
         ID_EX->instruction.immediate = 0;
         ID_EX->rsContents = 0;
         ID_EX->rtContents = 0;
         ID_EX->regResult = 0;
         return 0;
      }
    }
    //go in here if addi
    if(IF_ID->instruction.opcode == ADDI)
    {
      if(registers[IF_ID->instruction.rs].flag)
      {
        ID_EX->instruction = IF_ID->instruction;// UPDATES INSTRUCTION TO OTHER LATCH
        ID_EX->rsContents = registers[IF_ID->instruction.rs].contents; //pass reg contents to variable
        ID_EX->rtContents = 0;
        ID_EX->regResult = 0;
        registers[ID_EX->instruction.rt].flag = 0; // NOT SAFE TO USE ANYMORE
        return 1;
      }
      else
      {//send NOP add $0, $s0, $s0
         ID_EX->instruction.opcode = ADD;
         ID_EX->instruction.rs = 0;
         ID_EX->instruction.rt = 0;
         ID_EX->instruction.rd = 0;
         ID_EX->instruction.funct = ADDF;
         ID_EX->instruction.immediate = 0;
         ID_EX->rsContents = 0;
         ID_EX->rtContents = 0;
         ID_EX->regResult = 0;
         return 0;
      }
    }

    if(IF_ID->instruction.opcode == BEQ)
    {
      if(registers[IF_ID->instruction.rs].flag && registers[IF_ID->instruction.rt].flag)
      {
        branch = 0;  //branch detected, freeze IF
        ID_EX->instruction = IF_ID->instruction;
        ID_EX->rsContents = registers[IF_ID->instruction.rs].contents;//pass reg contents to variable
        ID_EX->rtContents = registers[IF_ID->instruction.rt].contents;//pass reg contents to variable
        ID_EX->regResult = 0; // clears old regResult
        return 1;
      }
      else
      {//send NOP add $0, $s0, $s0
        ID_EX->instruction.opcode = ADD;
        ID_EX->instruction.rs = 0;
        ID_EX->instruction.rt = 0;
        ID_EX->instruction.rd = 0;
        ID_EX->instruction.funct = ADDF;
        ID_EX->instruction.immediate = 0;
        ID_EX->rsContents = 0;
        ID_EX->rtContents = 0;
        ID_EX->regResult = 0;
        return 0;
      }
    }

    if(IF_ID->instruction.opcode == SW)
    {
      if(registers[IF_ID->instruction.rs].flag && registers[IF_ID->instruction.rt].flag)
      {
        ID_EX->instruction = IF_ID->instruction;
        ID_EX->rsContents = registers[IF_ID->instruction.rs].contents;//pass reg contents to variable
        ID_EX->rtContents = registers[IF_ID->instruction.rt].contents;//pass reg contents to variable
        ID_EX->regResult = 0; // clears old regResult
        return 1;
      }
      else
      {
        ID_EX->instruction.opcode = ADD;
        ID_EX->instruction.rs = 0;
        ID_EX->instruction.rt = 0;
        ID_EX->instruction.rd = 0;
        ID_EX->instruction.funct = ADDF;
        ID_EX->instruction.immediate = 0;
        ID_EX->rsContents = 0;
        ID_EX->rtContents = 0;
        ID_EX->regResult = 0;
        return 0;
      }
    }

    if(IF_ID->instruction.opcode == LW)
    {
      if(registers[IF_ID->instruction.rs].flag && registers[IF_ID->instruction.rt].flag)
      {
        ID_EX->instruction = IF_ID->instruction;
        ID_EX->rsContents = registers[IF_ID->instruction.rs].contents;//pass reg contents to variable
        ID_EX->rtContents = registers[IF_ID->instruction.rt].contents;//pass reg contents to variable
        ID_EX->regResult = 0; // clears old regResult
        registers[ID_EX->instruction.rt].flag = 0; // NOT SAFE TO USE ANYMORE
        return 1;
      }
      else
      {
        ID_EX->instruction.opcode = ADD;
        ID_EX->instruction.rs = 0;
        ID_EX->instruction.rt = 0;
        ID_EX->instruction.rd = 0;
        ID_EX->instruction.funct = ADDF;
        ID_EX->instruction.immediate = 0;
        ID_EX->rsContents = 0;
        ID_EX->rtContents = 0;
        ID_EX->regResult = 0;
        return 0;
      }
    }
    return 1;
}

int EX(struct latch* ID_EX, struct latch* EX_MEM, int mulcycles, int cycles)
{
    if(ID_EX->instruction.opcode == -1)// if haltSimulation
    {
        EX_MEM->instruction.opcode = -1;
        EX_MEM->instruction.rs = -1;
        EX_MEM->instruction.rt = -1;
        EX_MEM->instruction.rd = -1;
        EX_MEM->instruction.immediate = -1;
        return 0;
    }
    exCounter++;
    if(ID_EX->numCycles == 0)
    {
        if((ID_EX->instruction.opcode == MUL) && (ID_EX->instruction.funct == MULF))
            ID_EX->numCycles = mulcycles;
        else
            ID_EX->numCycles = cycles;
    }

    if(ID_EX->instruction.opcode == ADD || ID_EX->instruction.opcode == SUB || ID_EX->instruction.opcode == MUL)//For add, sub, mul
    {
        if(ID_EX->instruction.funct == ADDF) //we're in add
            ID_EX->regResult = ID_EX->rsContents + ID_EX->rtContents;
        else if(ID_EX->instruction.funct == SUBF)//we're in sub
            ID_EX->regResult = ID_EX->rsContents - ID_EX->rtContents;
        else if(ID_EX->instruction.funct == MULF)//we're in mul
        {
            int mask = createMask(0,31); // creates a mask for first 32 bits
            //multiply rs and rt and put least 32 significant bits in rd
            ID_EX->regResult = (ID_EX->rsContents * ID_EX->rtContents) & mask;
        }
        EX_MEM->instruction = ID_EX->instruction;
        EX_MEM->rsContents =  ID_EX->rsContents;
        EX_MEM->rtContents = ID_EX->rtContents;
        EX_MEM->regResult = ID_EX->regResult;
    }
    else if(ID_EX->instruction.opcode == ADDI)
    {
        ID_EX->regResult = ID_EX->rsContents + ID_EX->instruction.immediate;
        EX_MEM->instruction = ID_EX->instruction;
        EX_MEM->rsContents = ID_EX->rsContents;
        EX_MEM->rtContents = ID_EX->regResult;
        EX_MEM->regResult = ID_EX->regResult;
    }
   else if(ID_EX->instruction.opcode == BEQ)
    {
        branch = 1; //branch determined
        if(ID_EX->rsContents - ID_EX->rtContents == 0)
          PC = PC + ID_EX->instruction.immediate;

        EX_MEM->instruction = ID_EX->instruction;
        EX_MEM->rsContents = ID_EX->rsContents;
        EX_MEM->rtContents = ID_EX->rtContents;
        EX_MEM->regResult = ID_EX->regResult;
    }
    else //keep the values from the previous latch (lw,sw)
    {
      EX_MEM->instruction = ID_EX->instruction;
      EX_MEM->rsContents = ID_EX->rsContents;
      EX_MEM->rtContents = ID_EX->rtContents;
      EX_MEM->regResult = ID_EX->regResult;
    }

    ID_EX->numCycles--;
    if(ID_EX->numCycles == 0)
        return 1;
    else // should never go here as we are not doing batch
        return 0;
}

int MEM(struct latch* EX_MEM, struct latch* MEM_WB, int cycles)
{
    if(EX_MEM->instruction.opcode == -1)// for haltSimulation
    {
        MEM_WB->instruction = EX_MEM->instruction;
        return 0;
    }
    memCounter++;
    if(EX_MEM->numCycles == 0)
    {
        if(EX_MEM->instruction.opcode == LW || EX_MEM->instruction.opcode==SW)
            EX_MEM->numCycles = cycles;
        else
            EX_MEM->numCycles = 1;
    }

    if(EX_MEM->instruction.opcode == LW)
    {
      EX_MEM->rtContents = dataMemory[EX_MEM->rsContents + (EX_MEM->instruction.immediate)/4];
      MEM_WB->instruction = EX_MEM->instruction;
      MEM_WB->rsContents = EX_MEM->rsContents;
      MEM_WB->rtContents = EX_MEM->rtContents;
      MEM_WB->regResult = EX_MEM->regResult;
    }
    else if(EX_MEM->instruction.opcode == SW)
    {
      dataMemory[EX_MEM->rsContents + (EX_MEM->instruction.immediate)/4] = EX_MEM->rtContents;
      MEM_WB->instruction = EX_MEM->instruction;
      MEM_WB->rsContents = EX_MEM->rsContents;
      MEM_WB->rtContents = EX_MEM->rtContents;
      MEM_WB->regResult = EX_MEM->regResult;
    }
    else
    {
      MEM_WB->instruction = EX_MEM->instruction;
      MEM_WB->rsContents = EX_MEM->rsContents;
      MEM_WB->rtContents = EX_MEM->rtContents;
      MEM_WB->regResult = EX_MEM->regResult;
    }

    EX_MEM->numCycles--;
    if(EX_MEM->numCycles == 0)
        return 1;
    else
        return 0;
}

int WB(struct latch* MEM_WB)
{
    if(MEM_WB->instruction.opcode == -1)//haltSimulation
        return 0;
    wbCounter++;

    if(MEM_WB->instruction.opcode == ADD && MEM_WB->instruction.funct == ADDF)//For add, sub, mul
    {
        registers[MEM_WB->instruction.rd].contents = MEM_WB->regResult; //Writing the value calculated in EX back to rd
        registers[MEM_WB->instruction.rd].flag = 1; //data is valid
    }
    else if(MEM_WB->instruction.opcode == SUB && MEM_WB->instruction.funct == SUBF)//For add, sub, mul
    {
        registers[MEM_WB->instruction.rd].contents = MEM_WB->regResult; //Writing the value calculated in EX back to rd
        registers[MEM_WB->instruction.rd].flag = 1; //data is valid
    }
    else if(MEM_WB->instruction.opcode == MUL && MEM_WB->instruction.funct == MULF)//For add, sub, mul
    {
        registers[MEM_WB->instruction.rd].contents = MEM_WB->regResult; //Writing the value calculated in EX back to rd
        registers[MEM_WB->instruction.rd].flag = 1; //data is valid
    }
    else if(MEM_WB->instruction.opcode == ADDI) // case for addi
    {
        registers[MEM_WB->instruction.rt].contents = MEM_WB->regResult; //Writing the value calculated in EX back to rt
        registers[MEM_WB->instruction.rt].flag = 1; //data is valid
    }
    else if(MEM_WB->instruction.opcode == LW)
    {
        registers[MEM_WB->instruction.rt].contents = MEM_WB->rtContents;  //Writing the value taken from memory to rt
        registers[MEM_WB->instruction.rt].flag = 1; //data is valid
    }
    return 1;  //writeback is valid
}

//helper function for multiply
int createMask(int a, int b)// a=startbit b=endbit
{
    int i;
    int mask = 0;
    for (i=a; i<=b; i++)
        mask |= 1 << i;
    return mask;
}

int STRCMP (const char *p1, const char *p2)
{
    const unsigned char *s1 = (const unsigned char *) p1;
    const unsigned char *s2 = (const unsigned char *) p2;
    unsigned char c1, c2;
    do
    {
        c1 = (unsigned char) *s1++;
        c2 = (unsigned char) *s2++;
        if (c1 == '\0')
            return c1 - c2;
    }
    while (c1 == c2);
    return c1 - c2;
}

char* STRCAT(char* destination, const char* source)
{
	// make ptr point to the end of destination string
	char* ptr = destination + strlen(destination);
	// Appends characters of source to the destination string
	while (*source != '\0')
		*ptr++ = *source++;
	// null terminate destination string
	*ptr = '\0';
	// destination is returned by standard STRCAT()
	return destination;
}

char* STRSTR(char *str, char *substr)
{
	  while (*str)
	  {
		    char *Begin = str;
		    char *pattern = substr;
		    // If first character of sub string match, check for whole string
		    while (*str && *pattern && *str == *pattern)
			{
			      str++;
			      pattern++;
		   }
		    // If complete sub string match, return starting address
		    if (!*pattern)
		    	  return Begin;
		    str = Begin + 1;	// Increament main string
	  }
	  return NULL;
}
