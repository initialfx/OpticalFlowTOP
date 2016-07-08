/* Shared Use License: This file is owned by Derivative Inc. (Derivative) and
 * can only be used, and/or modified for use, in conjunction with 
 * Derivative's TouchDesigner software, and only if you are a licensee who has
 * accepted Derivative's TouchDesigner license or assignment agreement (which
 * also govern the use of this file).  You may share a modified version of this
 * file with another authorized licensee of Derivative's TouchDesigner software.
 * Otherwise, no redistribution or sharing of this file, with or without
 * modification, is permitted.
 */

#include "OpticalFlowTOP.h"

#include <windows.h>

#include <stdio.h>
#include <string.h>
#include <gl/gl.h>
#include <assert.h>


// These functions are basic C function, which the DLL loader can find
// much easier than finding a C++ Class.
// The DLLEXPORT prefix is needed so the compile exports these functions from the .dll
// you are creating
extern "C"
{

DLLEXPORT
int
GetTOPAPIVersion(void)
{
	// Always return TOP_CPLUSPLUS_API_VERSION in this function.
	return TOP_CPLUSPLUS_API_VERSION;
}

DLLEXPORT
TOP_CPlusPlusBase*
CreateTOPInstance(const OP_NodeInfo* info, TOP_Context *context)
{
	// Return a new instance of your class every time this is called.
	// It will be called once per TOP that is using the .dll
	return new OpenGLTOP(info);
}

DLLEXPORT
void
DestroyTOPInstance(TOP_CPlusPlusBase* instance, TOP_Context *context)
{
	// Delete the instance here, this will be called when
	// Touch is shutting down, when the TOP using that instance is deleted, or
	// if the TOP loads a different DLL
	delete (OpenGLTOP*)instance;
}

};

OpenGLTOP::OpenGLTOP(const OP_NodeInfo* info) : myNodeInfo(info)
{
	drawWidth = 1280;
	drawHeight = 720;
	int flowWidth = drawWidth / 4;
	int flowHeight = drawHeight / 4;

	// opticalFlow.setup(flowWidth, flowHeight);
	myRotation = 0.0;
	myExecuteCount = 0;
	ofSetupOpenGL(&myWindow, 0, 0, OF_WINDOW);

}

OpenGLTOP::~OpenGLTOP()
{

}

void
OpenGLTOP::getGeneralInfo(TOP_GeneralInfo* ginfo)
{
	// Uncomment this line if you want the TOP to cook every frame even
	// if none of it's inputs/parameters are changing.
	ginfo->cookEveryFrame = true;
}

bool
OpenGLTOP::getOutputFormat(TOP_OutputFormat* format)
{
	// In this function we could assign variable values to 'format' to specify
	// the pixel format/resolution etc that we want to output to.
	// If we did that, we'd want to return true to tell the TOP to use the settings we've
	// specified.
	// In this example we'll return false and use the TOP's settings
	format->width = drawWidth;
	format->height = drawHeight;
	return true;
}


void
OpenGLTOP::execute(const TOP_OutputFormatSpecs* outputFormat ,
							OP_Inputs* inputs,
							TOP_Context* context)
{
	myExecuteCount++;
	bool doUseInput = inputs->getNumInputs() > 0;
	ofTexture texture;
	if (doUseInput) {
		const OP_TOPInput *topInput = inputs->getInputTOP(0);
		texture.setUseExternalTextureID(topInput->textureIndex);
		texture.texData.width = topInput->width;
		texture.texData.height = topInput->height;
		texture.texData.tex_w = topInput->width;
		texture.texData.tex_h = topInput->height;
		texture.texData.textureTarget = topInput->textureType;
	}

	context->beginGLCommands();

	ofSetupOpenGL(&myWindow, 0, 0, OF_WINDOW);

	if (doUseInput) {
		texture.draw(0, 0);
	} else {
		ofSetColor(255, 0, 0);
		ofDrawRectangle(0, 0, drawWidth, drawHeight);
	}

	context->endGLCommands();
}

int
OpenGLTOP::getNumInfoCHOPChans()
{
	// We return the number of channel we want to output to any Info CHOP
	// connected to the TOP. In this example we are just going to send one channel.
	return 2;
}

void
OpenGLTOP::getInfoCHOPChan(int index,
										OP_InfoCHOPChan* chan)
{
	// This function will be called once for each channel we said we'd want to return
	// In this example it'll only be called once.

	if (index == 0)
	{
		chan->name = "executeCount";
		chan->value = (float)myExecuteCount;
	}

	if (index == 1)
	{
		chan->name = "rotation";
		chan->value = (float)myRotation;
	}
}

bool		
OpenGLTOP::getInfoDATSize(OP_InfoDATSize* infoSize)
{
	infoSize->rows = 2;
	infoSize->cols = 2;
	// Setting this to false means we'll be assigning values to the table
	// one row at a time. True means we'll do it one column at a time.
	infoSize->byColumn = false;
	return true;
}

void
OpenGLTOP::getInfoDATEntries(int index,
										int nEntries,
										OP_InfoDATEntries* entries)
{
	// It's safe to use static buffers here because Touch will make it's own
	// copies of the strings immediately after this call returns
	// (so the buffers can be reuse for each column/row)
	static char tempBuffer1[4096];
	static char tempBuffer2[4096];

	if (index == 0)
	{
		// Set the value for the first column
		strcpy_s(tempBuffer1, "executeCount");
		entries->values[0] = tempBuffer1;

		// Set the value for the second column
		sprintf_s(tempBuffer2, "%d", myExecuteCount);
		entries->values[1] = tempBuffer2;
	}

	if (index == 1)
	{
		// Set the value for the first column
		strcpy_s(tempBuffer1, "rotation");
		entries->values[0] = tempBuffer1;

		// Set the value for the second column
		sprintf_s(tempBuffer2, "%g", myRotation);
		entries->values[1] = tempBuffer2;
	}
}

void
OpenGLTOP::setupParameters(OP_ParameterManager* manager)
{
	// color 1
	{
		OP_NumericParameter	np;

		np.name = "Color1";
		np.label = "Color 1";

		for (int i=0; i<3; i++)
		{
			np.defaultValues[i] = 1.0;
			np.minValues[i] = 0.0;
			np.maxValues[i] = 1.0;
			np.minSliders[i] = 0.0;
			np.maxSliders[i] = 1.0;
			np.clampMins[i] = true;
			np.clampMaxes[i] = true;
		}
		
		ParAppendResult res = manager->appendRGB(np);
		assert(res == PARAMETER_APPEND_SUCCESS);
	}

	// color 2
	{
		OP_NumericParameter	np;

		np.name = "Color2";
		np.label = "Color 2";

		for (int i=0; i<3; i++)
		{
			np.defaultValues[i] = 0.0;
			np.minValues[i] = 0.0;
			np.maxValues[i] = 1.0;
			np.minSliders[i] = 0.0;
			np.maxSliders[i] = 1.0;
			np.clampMins[i] = true;
			np.clampMaxes[i] = true;
		}
		
		ParAppendResult res = manager->appendRGB(np);
		assert(res == PARAMETER_APPEND_SUCCESS);
	}

	// speed
	{
		OP_NumericParameter	np;

		np.name = "Speed";
		np.label = "Speed";
		np.defaultValues[0] = 1.0;
		np.minSliders[0] = -10.0;
		np.maxSliders[0] =  10.0;
		
		ParAppendResult res = manager->appendFloat(np);
		assert(res == PARAMETER_APPEND_SUCCESS);
	}

	// pulse
	{
		OP_NumericParameter	np;

		np.name = "Reset";
		np.label = "Reset";
		
		ParAppendResult res = manager->appendPulse(np);
		assert(res == PARAMETER_APPEND_SUCCESS);
	}

}

void
OpenGLTOP::pulsePressed(const char* name)
{
	if (!strcmp(name, "Reset"))
	{
		myRotation = 0.0;
	}


}
