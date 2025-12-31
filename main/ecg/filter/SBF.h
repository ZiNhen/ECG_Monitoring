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

// Begin header file, SBF.h

#ifndef SBF_H_ // Include guards
#define SBF_H_

/*
Generated code is based on the following filter design:
<micro.DSP.FilterDocument sampleFrequency="#500" arithmetic="float" biquads="Direct1" classname="SBF" inputMax="#1" inputShift="#-1" >
  <micro.DSP.IirButterworthFilter N="#4" bandType="s" w1="#0.08" w2="#0.1" stopbandRipple="#undefined" passbandRipple="#undefined" transitionRatio="#undefined" >
    <micro.DSP.FilterStructure coefficientBits="#0" variableBits="#0" accumulatorBits="#0" biquads="Direct1" >
      <micro.DSP.FilterSection form="Direct1" historyType="Double" accumulatorBits="#0" variableBits="#0" coefficientBits="#0" />
      <micro.DSP.FilterSection form="Direct1" historyType="Double" accumulatorBits="#0" variableBits="#0" coefficientBits="#0" />
      <micro.DSP.FilterSection form="Direct1" historyType="Double" accumulatorBits="#0" variableBits="#0" coefficientBits="#0" />
      <micro.DSP.FilterSection form="Direct1" historyType="Double" accumulatorBits="#0" variableBits="#0" coefficientBits="#0" />
    </micro.DSP.FilterStructure>
    <micro.DSP.PoleOrZeroContainer >
      <micro.DSP.PoleOrZero i="#0.47465172414628165" r="#0.8555988114670793" isPoint="#true" isPole="#true" isZero="#false" symmetry="c" N="#1" cascade="#3" />
      <micro.DSP.PoleOrZero i="#0.5681286974519106" r="#0.7913107471103241" isPoint="#true" isPole="#true" isZero="#false" symmetry="c" N="#1" cascade="#2" />
      <micro.DSP.PoleOrZero i="#0.48291838024546785" r="#0.8130055135746305" isPoint="#true" isPole="#true" isZero="#false" symmetry="c" N="#1" cascade="#1" />
      <micro.DSP.PoleOrZero i="#0.5192840280120979" r="#0.7852160061841602" isPoint="#true" isPole="#true" isZero="#false" symmetry="c" N="#1" cascade="#0" />
      <micro.DSP.PoleOrZero i="#0.5331871669114575" r="#0.845997307939531" isPoint="#true" isPole="#false" isZero="#true" symmetry="c" N="#1" cascade="#3" />
      <micro.DSP.PoleOrZero i="#0.5331871669114575" r="#0.845997307939531" isPoint="#true" isPole="#false" isZero="#true" symmetry="c" N="#1" cascade="#1" />
      <micro.DSP.PoleOrZero i="#0.5331871669114575" r="#0.845997307939531" isPoint="#true" isPole="#false" isZero="#true" symmetry="c" N="#1" cascade="#0" />
      <micro.DSP.PoleOrZero i="#0.5331871669114575" r="#0.845997307939531" isPoint="#true" isPole="#false" isZero="#true" symmetry="c" N="#1" cascade="#2" />
    </micro.DSP.PoleOrZeroContainer>
    <micro.DSP.GenericC.CodeGenerator generateTestCases="#false" />
    <micro.DSP.GainControl magnitude="#1" frequency="#0.45703125" peak="#true" />
  </micro.DSP.IirButterworthFilter>
</micro.DSP.FilterDocument>

*/

static const int SBF_numStages = 4;
static const int SBF_coefficientLength = 20;
extern float SBF_coefficients[20];

typedef struct
{
	float state[16];
	float output;
} SBFType;

typedef struct
{
    float *pInput;
    float *pOutput;
    float *pState;
    float *pCoefficients;
    short count;
} SBF_executionState;


SBFType *SBF_create( void );
void SBF_destroy( SBFType *pObject );
void SBF_init( SBFType * pThis );
void SBF_reset( SBFType * pThis );
#define SBF_writeInput( pThis, input )  \
    SBF_filterBlock( pThis, &(input), &(pThis)->output, 1 );

#define SBF_readOutput( pThis )  \
    (pThis)->output

int SBF_filterBlock( SBFType * pThis, float * pInput, float * pOutput, unsigned int count );
#define SBF_outputToFloat( output )  \
    (output)

#define SBF_inputFromFloat( input )  \
    (input)

void SBF_filterBiquad( SBF_executionState * pExecState );
#endif // SBF_H_
	
