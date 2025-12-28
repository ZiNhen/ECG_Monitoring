/******************************* SOURCE LICENSE *********************************
Copyright (c) 2021 MicroModeler.

A non-exclusive, nontransferable, perpetual, royalty-free license is granted to the Licensee to 
use the following Information for academic, non-profit, or government-sponsored research purposes.
Use of the following Information under this License is restricted to NON-COMMERCIAL PURPOSES ONLY.
Commercial use of the following Information requires a separately executed written license agreement.

This Information is distributed WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

******************************* END OF LICENSE *********************************/

// A commercial license for MicroModeler DSP can be obtained at https://www.micromodeler.com/launch.jsp

// Begin header file, LPF.h

#ifndef LPF_H_ // Include guards
#define LPF_H_

/*
Generated code is based on the following filter design:
<micro.DSP.FilterDocument sampleFrequency="#500" arithmetic="float" biquads="Direct1" classname="LPF" inputMax="#1" inputShift="#15" >
  <micro.DSP.IirButterworthFilter N="#4" bandType="l" w1="#0.08" w2="#0.4" stopbandRipple="#undefined" passbandRipple="#undefined" transitionRatio="#undefined" >
    <micro.DSP.FilterStructure coefficientBits="#0" variableBits="#0" accumulatorBits="#0" biquads="Direct1" >
      <micro.DSP.FilterSection form="Direct1" historyType="Double" accumulatorBits="#0" variableBits="#0" coefficientBits="#0" />
      <micro.DSP.FilterSection form="Direct1" historyType="Double" accumulatorBits="#0" variableBits="#0" coefficientBits="#0" />
    </micro.DSP.FilterStructure>
    <micro.DSP.PoleOrZeroContainer >
      <micro.DSP.PoleOrZero i="#0.37580016110834147" r="#0.7398994471986083" isPoint="#true" isPole="#true" isZero="#false" symmetry="c" N="#1" cascade="#1" />
      <micro.DSP.PoleOrZero i="#0.12757691517314285" r="#0.6064060463101093" isPoint="#true" isPole="#true" isZero="#false" symmetry="c" N="#1" cascade="#0" />
      <micro.DSP.PoleOrZero i="#0" r="#-1" isPoint="#true" isPole="#false" isZero="#true" symmetry="r" N="#1" cascade="#0" />
      <micro.DSP.PoleOrZero i="#0" r="#-1" isPoint="#true" isPole="#false" isZero="#true" symmetry="r" N="#1" cascade="#0" />
      <micro.DSP.PoleOrZero i="#0" r="#-1" isPoint="#true" isPole="#false" isZero="#true" symmetry="r" N="#1" cascade="#1" />
      <micro.DSP.PoleOrZero i="#0" r="#-1" isPoint="#true" isPole="#false" isZero="#true" symmetry="r" N="#1" cascade="#1" />
    </micro.DSP.PoleOrZeroContainer>
    <micro.DSP.GenericC.CodeGenerator generateTestCases="#false" />
    <micro.DSP.GainControl magnitude="#1" frequency="#0" peak="#true" />
  </micro.DSP.IirButterworthFilter>
</micro.DSP.FilterDocument>

*/

static const int LPF_numStages = 2;
static const int LPF_coefficientLength = 10;
extern float LPF_coefficients[10];

typedef struct
{
	float state[8];
	float output;
} LPFType;

typedef struct
{
    float *pInput;
    float *pOutput;
    float *pState;
    float *pCoefficients;
    short count;
} LPF_executionState;


LPFType *LPF_create( void );
void LPF_destroy( LPFType *pObject );
void LPF_init( LPFType * pThis );
void LPF_reset( LPFType * pThis );
#define LPF_writeInput( pThis, input )  \
    LPF_filterBlock( pThis, &(input), &(pThis)->output, 1 );

#define LPF_readOutput( pThis )  \
    (pThis)->output

int LPF_filterBlock( LPFType * pThis, float * pInput, float * pOutput, unsigned int count );
#define LPF_outputToFloat( output )  \
    (output)

#define LPF_inputFromFloat( input )  \
    (input)

void LPF_filterBiquad( LPF_executionState * pExecState );
#endif // LPF_H_
	
