#ifndef HANDYPSP

extern "C" {
#include "65C02.h"
}

DWORD     autoboot          = 0;
BOOL      restart           = 0;
//WORD      iorange           = 0x0040;

BOOL      g_irq             = 0;    // FIXME: NO MORE REVERSE
BOOL      g_nmi             = 0;    // FIXME: NO MORE REVERSE
BOOL      g_stp             = 0;
BOOL      g_wai             = 0;



//void ClearCounters() {
//    stmsecs = GetTickCount();
//    totcycles = 0;
//    executed = 0;
//}


// extern WORD LogDisassembly (WORD offset, LPTSTR text);

// extern bool timer0started;
// extern bool timer1started;

// void ContinueExecution () {
//
//     DWORD processtime        = totcycles;// needed for comm routines
//     DWORD loop               = speed * 300;// watchdog timer
//     DWORD elapsed            = 0;
//     DWORD CpuTicks          = 0;
//     DWORD j = 0;
//
//     if (benchmark == 2) {
//         ClearCounters();
//         benchmark = 1;
//     }
//
//     elapsed = GetTickCount()-stmsecs;
//
//     while (loop && (totcycles < (elapsed * 100 * speed))) {
//         //qDebug("PC:0x%04x, opcode: 0x%06x", regs.pc, (*(LPDWORD)(mem+regs.pc)) & 0xFFFFFF);
//         //LogDisassembly(regs.pc, NULL);
//         CpuTicks = CpuExecute();
//         totcycles += CpuTicks;
//         executed++;
//         //*** add checks for reset, IRQ, NMI, and other pin signals
//         elapsed = GetTickCount()-stmsecs;
//         loop--;
//         /* added for irq timing */
//         if (irqclk) {
//             irqcnt += CpuTicks;
//             if (irqcnt >= irqclk) {
//                 irq=0;
//                 irqcnt = irqcnt - irqclk;
//             }
//         }
//         if (nmiclk) {
//             nmicnt += CpuTicks;
//             if (nmicnt >= nmiclk) {
//                 nmi=0;
//                 nmicnt = nmicnt - nmiclk;
//             }
//         }
//         if (timer0started) {
//             mem[02] = mem[02] + 1;
//         }
//         if (timer1started) {
//             mem[03] = mem[03] + 1;
//         }
//
//         /* Throttling routine (simple delay loop)  */
//         if (throttle) for (j=throttle*CpuTicks;j>1;j--);
//
//     }
//     /* Throttling update routine   */
//     if (throttle) {
//         if (totcycles < (elapsed * 100 * speed)) {
//             throttle--;
//             if (throttle < 1) throttle = 1;
//         }
//         else {
//             throttle++;
//         }
//     }
//
//
// //     if (benchmark && (elapsed >= 5000)) {
// //         DisplayBenchmark(elapsed);
// //         benchmark = 0;
// //         ClearCounters();
// //     }
//
// //     DrawStatusArea((HDC)0,0);
// //     if (Displayflag) TerminalDisplay(0);
// //     CommUpdate(totcycles - processtime);
//
// }


#endif
