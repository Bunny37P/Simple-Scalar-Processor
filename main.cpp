#include <bits/stdc++.h>
#include<fstream>
using namespace std;

bool Regs[16];          //validity bits for registers
int8_t RF[16];          //register file
int8_t DCache[256];     //Data Cache
int ICache[256];        //Instruction
int PC=0;               //program counter
int num_load=0;         
int num_store=0;
int num_stall=0;
int num_dat_st=0;
int num_con_st=0;
int num_inst=0;
int num_cycles=0;
int num_arith =0;
int num_log=0;
int num_con=0;
int num_halt=0;
int num_shift=0;
int halt=0;             //halt 
int ctrl_stall=1;       //flags for stalls
int to4_bit(int n)
{
    // array to store binary number
    if(n/8>0){
        return n-16;
    }
    return n;
}
class Instructs{
    private:
    int IR, args[4], IPC;       // Instruction register, helper program counter,
    int RA, RB, Imm, ALU_out, Cond;     //source registers, ALU output
    int8_t Imm_8;                  
    int LMD;                        //Load Memory data
    int is_stalled=0;               //Flag for stalls
    
    public:
    int stage;          //instruction stages

    int get_ins(int check) {
        if(!Regs[check]){               //get register value
            is_stalled=1;              
        }else{
            is_stalled=0;               
        }
        return is_stalled;

    }
    void get_args(){
        int temp = IR;                      
        for(int i=3; i>=0;i--){
            args[i]= temp & 15;
            temp = (temp >> 4);
        }
    }

    void Fetch_it(){            //Fetch part
        IPC =PC;
        IR = (ICache[IPC] << 8) + ICache[IPC + 1];
        IPC+=2;
        PC=IPC;
        stage =1;
    }

    void Decode_it(){           //decode part
        get_args();
        if(args[0]==0 ||args[0]==1 || args[0]==2 || args[0]==4 || args[0]==5 ||args[0]==6 ){ 
            //0 -> ADD, 1-> SUB , 2->MUL, 4-> AND, 5 ->OR, 6 -> XOR
            if(get_ins(args[2])){
                return;
            }
            if(get_ins(args[3])){
                return;
            }
            RA = static_cast<int8_t>(RF[args[2]]);
            RB= static_cast<int8_t>(RF[args[3]]);
            Regs[args[1]]= false;
        }else if(args[0]==3 ){          //INC
            if(get_ins(args[1])){
                return;
            }
            RA = static_cast<int8_t>(RF[args[1]]);
            Regs[args[1]] = false;
        }else if(args[0]==7 ){      //NOT
            if(get_ins(args[2])){
                return;
            }
            RA = static_cast<int8_t>(RF[args[1]]);
            Regs[args[1]] = false;
        }
        else if(args[0]==8 || args[0]==9){   //8 ->SLLI , 9-> SRLI
            if(get_ins(args[2])){
                return;
            }
            RA = static_cast<int8_t>(RF[args[2]]);
            Imm = args[3];
            Regs[args[1]]= false;
        }else if(args[0]==10){              // LI

        Imm = (args[2] << 4) + args[3];
        Regs[args[1]] = false;

        }else if(args[0]==11){          //LD
            if(get_ins(args[2])){
                return;
            }
            RA = static_cast<int8_t>(RF[args[2]]);
            Imm = to4_bit(args[3]);
            Regs[args[1]]= false;
        }
        else if(args[0]==12){           //ST
            if(get_ins(args[1])){
                return;
            }
            if(get_ins(args[2])){           
                return;
            }
            RA = static_cast<int8_t>(RF[args[1]]);
            RB = static_cast<int8_t>(RF[args[2]]);
            Imm = to4_bit(args[3]);
        }else if(args[0]==13){          //JMP
            Imm = (args[1] << 4) + args[2];
            ctrl_stall = 0;

        }else if(args[0]==14){      //BEQZ
            if(get_ins(args[1])){
                return;
            }
            RA= static_cast<int8_t>(RF[args[1]]);
            Imm = (args[2] << 4 ) + args[3];
            ctrl_stall =0;
        
        }else if( args[0] == 15){       //HALT
            halt =1;
        }
        stage++;
    }
    void Execute_it(){
        if(args[0]==0){
            ALU_out = RA + RB;
            Regs[args[1]] = false;
        }
        else if(args[0]==1){
            ALU_out = RA - RB;
            Regs[args[1]] = false;
        }
        else if(args[0]==2){
            ALU_out = RA * RB;
            Regs[args[1]] = false;
        }
        else if(args[0]==3){
            ALU_out = RA + 1;
            Regs[args[1]] = false;
        }
        else if(args[0]==4){
            ALU_out = RA & RB;
            Regs[args[1]] = false;
        }
        else if(args[0]==5){
            ALU_out = RA | RB;
            Regs[args[1]] = false;
        }
        else if(args[0]==6){
            ALU_out = RA ^ RB;
            Regs[args[1]] = false;
        }
        else if(args[0]==7){
            ALU_out = ~ RA;
            Regs[args[1]] = false;
        }
        else if(args[0]==8){
            ALU_out = RA << Imm;
            Regs[args[1]] = false;
        }
        else if(args[0]==9){
            ALU_out = RA >> Imm;
            Regs[args[1]] = false;
        }
        else if(args[0]==10){
            ALU_out = static_cast<int8_t>(Imm);
            Regs[args[1]] = false;
        }
        else if(args[0]==11){
            ALU_out = RA + Imm;
            Regs[args[1]] = false;
        }
        else if(args[0]==12){
            ALU_out = RB + Imm;
        }
        else if(args[0]==13){
             Imm_8 = Imm;
             ALU_out = IPC + Imm_8 * 2;
             PC = ALU_out;
        }
        else if(args[0]==14){
             Imm_8 = Imm;
             ALU_out = IPC + Imm_8 * 2;
             if(!RA)
             PC = ALU_out;
        }
        stage++;
    }


    void Mem_Access(){
     if(args[0]==10){
        LMD = Imm;
     }else if(args[0]==11){
        LMD = DCache[ALU_out];
     }else if(args[0]== 12){
        DCache[ALU_out]= RF[args[1]];
     }else if(args[0]==13 || args[0] == 14){
        ctrl_stall= 1;
     }
     stage ++;
    }


    void Write(){
        if(args[0] < 10){
            RF[args[1]] = ALU_out;
            Regs[args[1]] = 1;
        }else if(args[0] < 12){
            RF[args[1]] = LMD;
            //cout<<RF[args[1]]+0<<"hello"<<endl;
            Regs[args[1]] =1;
        }
        
        stage++;
        Count_it();
    }

    int Next_stage(){
        if(stage==1){
            Decode_it();
        }else if(stage==2){
            Execute_it();
        }else if(stage ==3){
            Mem_Access();
        }else if(stage ==4){
            Write();
        }
        return is_stalled;
    }

    void Count_it(){
        if(args[0] < 4){
            num_arith++;
        } else if(args[0] < 8){
            num_log ++;
        } else if(args[0] < 10){
            num_shift ++;
        } else if( args[0] < 11){
            num_load++;
        }else if (args[0] <13){
            num_store ++;
        } else if(args[0] < 15){
            num_con++;
        }else {
            num_halt++;
        }
    }

};


void Read_it(int8_t arr[], int size, string Naam){
    ifstream ifile;
    ifile.open(Naam);
    //cout<<ifile.is_open()<<endl;
    //cout <<Naam << endl;
    //cout<<size<<endl;
    string str;
    for(int i=0;i< size;i++){
        std::string Val;
        getline(ifile,Val);
        arr[i] = std::stoi(Val, NULL, 16);

    }
    ifile.close();
}
void Read_it2(int arr[], int size, string Naam){
    ifstream ifile;
    ifile.open(Naam);
    
    string str;
    for(int i=0;i< size;i++){
        string Val;
        ifile >> Val;
        arr[i] = stoi(Val, NULL, 16);

    }
    ifile.close();
}


void Print_it_out(int8_t arr[], int size, string filename)
{ 
    ofstream out;
    out.open(filename);
   // cout<<out.is_open()<<endl;
    for (int i = 0; i < size; i++)
    {
        int temp = arr[i];
        temp = temp & 0xff;
        out << hex << (temp >> 4);
        temp = temp & 0xf;
        out << hex << temp << endl;
    }
    out.close();
}


void Print_it(string filenaam){
    ofstream out;
    out.open(filenaam);
    out << "Total number of instructions executed    : " << num_inst << endl;
    out << "Number of instructions in each class"                    << endl;
    out << "Arithmetic instructions                  : " << num_arith << endl;
    out << "Logical instructions                     : " << num_log << endl;
    out << "Shift instructions                       : " << num_shift << endl;
    out << "Memory instructions                      : " << num_store << endl;
    out << "Load immediate instructions              : " << num_load << endl;
    out << "Control instructions                     : " << num_con << endl;
    out << "Halt instructions                        : " << num_halt << endl;
    out << "Cycles Per Instruction                   : " << (double)num_cycles / num_inst << endl;
    out << "Total number of stalls                   : " << num_stall << endl;
    out << "Data stalls (RAW)                        : " << num_dat_st << endl;
    out << "Control stalls                           : " << num_con_st << endl;
    out.close();
}



void Initialize_it(){
    for(int i=0; i<16; i++){
        Regs[i]=true;
    }
}

int main(){
    Read_it2(ICache, 256, "input/ICache.txt");
    Read_it(DCache, 256, "input/DCache.txt");
    Read_it(RF, 16, "input/RF.txt");
    queue<Instructs> Instr;
   // cout<<RF[0]+0<<endl;
    Initialize_it();
    //cout<<11<<endl;
        Instructs Ins1;
        Ins1.Fetch_it();
        Instr.push(Ins1);
    num_cycles++;
         num_inst ++;
    //cout<<12<<endl;
    while(!Instr.empty()){
        //cout<<2<<endl;
        num_cycles++;
        int stall_count=0;
        int N= Instr.size();
        for(int i=0; i<N ;i++){
            Instructs S= Instr.front();
            stall_count += S.Next_stage();
            if(S.stage < 5){
                Instr.push(S);
            }
            Instr.pop();
            
        }
        if(ctrl_stall==0){
            num_con_st++;
            num_stall++;
        }
        else if(stall_count > 0){
            num_dat_st++;
            num_stall++;
        }else if(halt == 0){
             num_inst ++;
            Instructs Ins;
            Ins.Fetch_it();
            Instr.push(Ins);
           
        }
        //cout<<"abc"<<endl;

    }
//cout<<1<<endl;
Print_it_out(DCache, 256, "output/DCache.txt");
Print_it("output/Output.txt");

}