#ifndef CPU_h
#define CPU_h

/* INCLUDE */
#include "types.h"

/* DEFINES */
/* STATUS REGISTER FLAGS */
#define SIGN 0x80
#define OVERFLOW 0x40
#define CONSTANT 0x20
#define BREAK 0x10
#define DECIMAL 0x08
#define INTERRUPT 0x04
#define ZERO 0x02
#define CARRY 0x01

/* STATUS REGISTER MACROS */
#define ISSET(_flag) (cpu->P & (_flag))
#define SET(_flag) (cpu->P |= (_flag))
#define CLEAR(_flag) (cpu->P &= ~(_flag))

/* ROMs */
extern uint8_t cycle_rom[0x100];
extern void (*instruction_rom[0x100])(CPU* cpu);
extern uint16_t (*addressing_rom[0x100])(CPU* cpu);

/* METHOD PROTOTYPES */
void cpu_init(CPU* cpu);
void cpu_step(CPU* cpu);
uint8_t fetch(CPU* cpu);

/* INSTRUCTION SET PROTOTYPES */
void ADC(CPU* cpu);
void AND(CPU* cpu);
void ASL(CPU* cpu);
void BCC(CPU* cpu);
void BCS(CPU* cpu);
void BEQ(CPU* cpu);
void BIT(CPU* cpu);
void BMI(CPU* cpu);
void BNE(CPU* cpu);
void BPL(CPU* cpu);
void BRK(CPU* cpu);
void BVC(CPU* cpu);
void BVS(CPU* cpu);
void CLC(CPU* cpu);
void CLD(CPU* cpu);
void CLI(CPU* cpu);
void CLV(CPU* cpu);
void CMP(CPU* cpu);
void CPX(CPU* cpu);
void CPY(CPU* cpu);
void DEC(CPU* cpu);
void DEX(CPU* cpu);
void DEY(CPU* cpu);
void EOR(CPU* cpu);
void INC(CPU* cpu);
void INX(CPU* cpu);
void INY(CPU* cpu);
void JMP(CPU* cpu);
void JSR(CPU* cpu);
void LDA(CPU* cpu);
void LDX(CPU* cpu);
void LDY(CPU* cpu);
void LSR(CPU* cpu);
void NOP(CPU* cpu);
void ORA(CPU* cpu);
void PHA(CPU* cpu);
void PHP(CPU* cpu);
void PLA(CPU* cpu);
void PLP(CPU* cpu);
void ROL(CPU* cpu);
void ROR(CPU* cpu);
void RTI(CPU* cpu);
void RTS(CPU* cpu);
void SBC(CPU* cpu);
void SEC(CPU* cpu);
void SED(CPU* cpu);
void SEI(CPU* cpu);
void STA(CPU* cpu);
void STX(CPU* cpu);
void STY(CPU* cpu);
void TAX(CPU* cpu);
void TAY(CPU* cpu);
void TSX(CPU* cpu);
void TXA(CPU* cpu);
void TYA(CPU* cpu);
void TXS(CPU* cpu);

#endif

