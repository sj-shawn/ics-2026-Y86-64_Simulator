#include "cpu_common.h"

/*
I_HALT       停机
I_NOP         nop
I_RRMOVQ      rrmovq rA rB     包含了cmov，要判断条件
I_IRMOVQ      irmovq V rB
I_RMMOVQ      rmmovq rA D(rB)
I_MRMOVQ      mrmovq D(rB) rA
I_ALU         op rA rB
I_JMP         jmp Dest
I_CALL        call Dest      需要把结果压回栈，要考虑rsp
I_RET         ret
I_PUSHQ       pushq rA
I_POPQ        popq rA   出栈后传入到rA中
*/

/* 每个阶段通过 CPU 结构体中的暂存值传递数据。请补全以下六个阶段。 */
void fetch(CPU *cpu)
{
    uint64_t byte0 = read_mem(cpu, cpu->pc, 1);
    if (cpu->stat != AOK) return;
    cpu->icode = (int)(byte0 >> 4);
    cpu->ifun = (int)(byte0 & 15);

    /* TODO: 校验指令和寄存器编码，处理 halt，读取寄存器字节和立即数，计算 valP。
       完成实现后删除下面的占位状态。 */

    int flag = 0;                      // 判断是否取出来的是非法码 
    int length = 0;                    // 指令的长度

    switch(cpu->icode){
        case I_HALT : {                                           
            if (cpu->ifun != 0){cpu->stat = INS; return;}                                             
            cpu->stat = HLT; 
            return;
        }                      
        case I_NOP : {                                        
            if (cpu->ifun != 0) {flag = 1; break;}
            length = 1;
            break;  
        }                                      
        case I_RRMOVQ : {                                         
            if (cpu->ifun > 6) {flag = 1; break;}
            length = 2;                                              
            uint64_t byte1 = read_mem(cpu, cpu->pc + 1, 1);
            if (cpu->stat != AOK) return;
            
            cpu->rA = (int)(byte1 >> 4);
            cpu->rB = (int)(byte1 & 15);
            if (cpu->rA == RNONE || cpu->rB == RNONE){flag = 1; break;}
            break;
        }
        case I_IRMOVQ : {                                       
            if (cpu->ifun != 0) {flag = 1; break;}
            length = 10;
            uint64_t byte1 = read_mem(cpu, cpu->pc + 1, 1);
            if (cpu->stat != AOK) return;
            
            cpu->rA = (int)(byte1 >> 4);
            cpu->rB = (int)(byte1 & 15);
            if(cpu->rA != RNONE || cpu->rB == RNONE){flag = 1; break;}

            uint64_t byte2 = read_mem(cpu, cpu->pc + 2, 8);
            if (cpu->stat != AOK) return;
            cpu->valC = byte2;
            break;
        }
        case I_RMMOVQ : {                                      
            if (cpu->ifun != 0) {flag = 1; break;}
            length = 10;
            uint64_t byte1 = read_mem(cpu, cpu->pc + 1, 1);
            if (cpu->stat != AOK) return;

            cpu->rA = (int)(byte1 >> 4);
            cpu->rB = (int)(byte1 & 15);
            if (cpu->rA == RNONE || cpu->rB == RNONE){flag = 1; break;}
            
            uint64_t byte2 = read_mem(cpu, cpu->pc + 2, 8);
            if (cpu->stat != AOK) return;
            cpu->valC = byte2;
            break;
        }
        case I_MRMOVQ : {                                
            if (cpu->ifun != 0) {flag = 1; break;}
            length = 10;
            uint64_t byte1 = read_mem(cpu, cpu->pc + 1, 1);
            if (cpu->stat != AOK) return;
            
            cpu->rA = (int)(byte1 >> 4);
            cpu->rB = (int)(byte1 & 15);
            if (cpu->rA == RNONE || cpu->rB == RNONE){flag = 1; break;}
            
            uint64_t byte2 = read_mem(cpu, cpu->pc + 2, 8);
            if (cpu->stat != AOK) return;
            cpu->valC = byte2;
            break;
        }
        case I_ALU : {                                         
            if (cpu->ifun > 3) {flag = 1; break;}    
            length = 2;                                          
            uint64_t byte1 = read_mem(cpu, cpu->pc + 1, 1);
            if (cpu->stat != AOK) return;
            
            cpu->rA = (int)(byte1 >> 4);
            cpu->rB = (int)(byte1 & 15);
            if (cpu->rA == RNONE || cpu->rB == RNONE){flag = 1; break;}
            break;
        }
        case I_JMP : {                               
            if (cpu->ifun > 6) {flag = 1; break;}
            length = 9;
            uint64_t byte1 = read_mem(cpu, cpu->pc + 1, 8);
            if (cpu->stat != AOK) return;
            cpu->valC = byte1; // 跳转的地址
            break;
        }
        case I_CALL : {                                        
            if (cpu->ifun != 0) {flag = 1; break;}
            length = 9;
            uint64_t byte1 = read_mem(cpu, cpu->pc + 1, 8);
            if (cpu->stat != AOK) return;
            cpu->valC = byte1;
            break;
        }
        case I_RET : {                                         
            if (cpu->ifun != 0) {flag = 1; break;}
            length = 1;
            break; 
        }
        case I_PUSHQ : {                                
            if (cpu->ifun != 0) {flag = 1; break;}
            length = 2;
            uint64_t byte1 = read_mem(cpu, cpu->pc + 1, 1);
            if (cpu->stat != AOK) return;
            
            cpu->rA = byte1 >> 4;
            cpu->rB = byte1 & 15;
            if(cpu->rB != RNONE || cpu->rA == RNONE){flag = 1; break;}
            break;
        }
        case I_POPQ : {                             
            if (cpu->ifun != 0) {flag = 1; break;}
            length = 2;
            uint64_t byte1 = read_mem(cpu, cpu->pc + 1, 1);
            if (cpu->stat != AOK) return;
            
            cpu->rA = byte1 >> 4;
            cpu->rB = byte1 & 15;
            if(cpu->rB != RNONE || cpu->rA == RNONE){flag = 1; break;}
            break;
        }
        default : {                                           // 非法操作码
            cpu->stat = INS;
            return;
        }
    }
     
    if (flag == 1){cpu->stat = INS; return;}
    cpu->valP = cpu->pc + length;                       // 下一条指令的地址
}

void decode(CPU *cpu)
{
    /* TODO: 根据指令选择源寄存器，将读取的值写入 valA 和 valB。
       栈操作隐式使用 RSP，RNONE 对应的值为 0。 */

    switch(cpu->icode){
        case I_NOP : {break;}
        case I_RRMOVQ : {cpu->valA = cpu->reg[cpu->rA]; cpu->valB = 0; break;}
        case I_IRMOVQ : {cpu->valA = 0; cpu->valB = 0; break;}
        case I_RMMOVQ : {cpu->valA = cpu->reg[cpu->rA]; cpu->valB = cpu->reg[cpu->rB]; break;}
        case I_MRMOVQ : {cpu->valA = 0; cpu->valB = cpu->reg[cpu->rB]; break;}
        case I_ALU : {cpu->valA = cpu->reg[cpu->rA]; cpu->valB = cpu->reg[cpu->rB]; break;}
        case I_JMP : {cpu->valA = 0; cpu->valB = 0; break;}
        case I_CALL : {cpu->valA = 0; cpu->valB = cpu->reg[RSP]; break;}   // 把返回地址压入栈
        case I_RET : {cpu->valA = cpu->reg[RSP]; cpu->valB = cpu->reg[RSP]; break;}
        case I_PUSHQ : {cpu->valA = cpu->reg[cpu->rA]; cpu->valB = cpu->reg[RSP]; break;}
        case I_POPQ : {cpu->valA = cpu->reg[RSP]; cpu->valB = cpu->reg[RSP]; break;} 
        default : break;
    }
}

void execute(CPU *cpu)
{
    /* TODO: 选择 ALU 操作数并计算 valE。OPq 更新 ZF、SF、OF。
       条件传送和跳转使用 check_cond 计算 cnd。使用无符号运算实现 64 位回绕。 */

    switch(cpu->icode){
        case I_NOP : {break;}
        case I_RRMOVQ : {cpu->valE = 0 + cpu->valA; cpu->cnd = check_cond(cpu, cpu->ifun); break;}
        case I_IRMOVQ : {cpu->valE = 0 + cpu->valC; break;}
        case I_RMMOVQ : {cpu->valE = cpu->valC + cpu->valB; break;}
        case I_MRMOVQ : {cpu->valE = cpu->valC + cpu->valB; break;}
        case I_ALU : {
            if (cpu->ifun == 0){
                cpu->valE = cpu->valA + cpu->valB;
                uint64_t sign_A = (cpu->valA >> 63) & 1;
                uint64_t sign_B = (cpu->valB >> 63) & 1;
                uint64_t sign_E = (cpu->valE >> 63) & 1;
                cpu->of = (!(sign_A ^ sign_B)) && (!(sign_A == sign_E));
            }
            else if (cpu->ifun == 1){
                cpu->valE = cpu->valB - cpu->valA;
                uint64_t sign_A = (cpu->valA >> 63) & 1;
                uint64_t sign_B = (cpu->valB >> 63) & 1;
                uint64_t sign_E = (cpu->valE >> 63) & 1;
                cpu->of = (sign_A ^ sign_B) && (sign_B != sign_E);
            }
            else if (cpu->ifun == 2){
                cpu->valE = cpu->valA & cpu->valB;
                cpu->of = 0;
            }
            else if (cpu->ifun == 3){
                cpu->valE = cpu->valA ^ cpu->valB;
                cpu->of = 0;
            }

            cpu->zf = cpu->valE == 0;
            cpu->sf = (((cpu->valE >> 63) & 1) == 1);

            break;
        }
        case I_JMP : {cpu->cnd = check_cond(cpu, cpu->ifun); break;}
        case I_CALL : {cpu->valE = cpu->valB - 8; break;}
        case I_RET : {cpu->valE = cpu->valB + 8; break;}
        case I_PUSHQ : {cpu->valE = cpu->valB - 8; break;}
        case I_POPQ : {cpu->valE = cpu->valB + 8; break;} 
        default : {cpu->valE = 0; cpu->cnd = 0; break;}
    }
}

void memory_stage(CPU *cpu)
{
    /* TODO: 根据指令访问内存。pushq 写入 valA，call 写入 valP。
       读取操作将结果写入 valM，使用 read_mem 和 write_mem 检查地址。 */
    
    switch(cpu->icode){
        case I_NOP : {break;}
        case I_RRMOVQ : {break;}
        case I_IRMOVQ : {break;}
        case I_RMMOVQ : {
            write_mem(cpu, cpu->valE, 8, cpu->valA); 
            break;
        }
        case I_MRMOVQ : {
            cpu->valM = read_mem(cpu, cpu->valE, 8);
            break;
        }
        case I_ALU : {break;}
        case I_JMP : {break;}
        case I_CALL : {
            write_mem(cpu, cpu->valE, 8, cpu->valP);
            break;
        }
        case I_RET : {
            cpu->valM = read_mem(cpu, cpu->valA, 8);
            break;
        }
        case I_PUSHQ : {
            write_mem(cpu, cpu->valE, 8, cpu->valA);
            break;
        }
        case I_POPQ : {
            cpu->valM = read_mem(cpu, cpu->valB, 8);
            break;
        } 
        default : break;
    }

}

void write_back(CPU *cpu)
{
    /* TODO: 选择 dstE、dstM 并写入寄存器。
       若两者均为 RSP，valM 应覆盖 valE。 */
    // if (cpu->stat != AOK) return;      memory_stage可能会越界

    switch(cpu->icode){
        case I_NOP : {break;}
        case I_RRMOVQ : {
            if (cpu->cnd) cpu->reg[cpu->rB] = cpu->valE;
            break;
        }
        case I_IRMOVQ : {
            cpu->reg[cpu->rB] = cpu->valE;
            break;
        }
        case I_RMMOVQ : {break;}
        case I_MRMOVQ : {
            cpu->reg[cpu->rA] = cpu->valM; 
            break;
        }
        case I_ALU : {
            cpu->reg[cpu->rB] = cpu->valE;
            break;
        }
        case I_JMP : {break;}
        case I_CALL : {
            cpu->reg[RSP] = cpu->valE;
            break;
        }
        case I_RET : {
            cpu->reg[RSP] = cpu->valE;       // 更改栈顶指针
            break;
        }
        case I_PUSHQ : {
            cpu->reg[RSP] = cpu->valE;
            break;
        }
        case I_POPQ : {
            cpu->reg[RSP] = cpu->valE;
            cpu->reg[cpu->rA] = cpu->valM;
            break;
        } 
        default : break;
    }
}

void update_pc(CPU *cpu)
{
    /* TODO: 按 call、条件跳转、ret 或顺序执行选择下一条指令的地址。 */
    switch(cpu->icode){
        case I_NOP : {cpu->pc = cpu->valP; break;}
        case I_RRMOVQ : {cpu->pc = cpu->valP; break;}
        case I_IRMOVQ : {cpu->pc = cpu->valP; break;}
        case I_RMMOVQ : {cpu->pc = cpu->valP; break;}
        case I_MRMOVQ : {cpu->pc = cpu->valP; break;}
        case I_ALU : {cpu->pc = cpu->valP; break;}
        case I_JMP : {
            if (cpu->cnd) cpu->pc = cpu->valC;
            else if (!cpu->cnd) cpu->pc = cpu->valP; 
            break;
        } 
        case I_CALL : {cpu->pc = cpu->valC; break;}   
        case I_RET : {cpu->pc = cpu->valM; break;} 
        case I_PUSHQ : {cpu->pc = cpu->valP; break;}
        case I_POPQ : {cpu->pc = cpu->valP; break;} 
        default : {cpu->pc = cpu->valP; break;}
    }
}
