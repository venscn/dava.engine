/*==================================================================================
Copyright (c) 2008, binaryzebra
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the binaryzebra nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "BaseTest.h"

const float32 BaseTest::FRAME_OFFSET = 5;

BaseTest::BaseTest(const String& _testName, uint32 frames, float32 delta, uint32 _debugFrame)
    :   frameNumber(0)
    ,   fixedDelta(delta)
    ,   targetFramesCount(frames)
    ,   targetTestTime(0)
    ,   testTime(0.0f)
    ,   startTime(0)
    ,   testName(_testName)
    ,   debugFrame(_debugFrame)
    ,   debuggable(false)

{
}

BaseTest::BaseTest(const String& _testName, uint32 _time)
    :   frameNumber(0)
    ,   fixedDelta(0.0f)
    ,   targetFramesCount(0)
    ,   targetTestTime(_time)
    ,   testTime(0.0f)
    ,   startTime(0)
    ,   testName(_testName)
    ,   debugFrame(0)
    ,   debuggable(false)

{
}

void BaseTest::LoadResources()
{
    const Size2i& size = DAVA::VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize();
    Rect viewport;

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.dx = size.dx;
    viewport.dy = size.dy;

    scene = new Scene();

    sceneView = new UI3DView(viewport, true);
    sceneView->SetScene(scene);

    AddControl(sceneView);
}

void BaseTest::UnloadResources()
{
    SafeRelease(scene);
}
void BaseTest::OnStart()
{
    Logger::Info(TeamcityTestsOutput::FormatTestStarted(testName).c_str());
}

void BaseTest::OnFinish()
{
    elapsedTime = SystemTimer::Instance()->FrameStampTimeMS() - startTime;

    float32 minDelta = FLT_MAX;
    float32 maxDelta = FLT_MIN;
    float32 averageDelta = 0.0f;

    float32 testTime = 0.0f;
    float32 elapsedTime = 0.0f;

    uint32 framesCount = GetFramesInfo().size();

    for (BaseTest::FrameInfo frameInfo : GetFramesInfo())
    {
        if (frameInfo.delta > maxDelta)
        {
            maxDelta = frameInfo.delta;
        }
        if (frameInfo.delta < minDelta)
        {
            minDelta = frameInfo.delta;
        }

        averageDelta += frameInfo.delta;
    }

    averageDelta /= framesCount;

    testTime = GetTestTime();
    elapsedTime = GetElapsedTime() / 1000.0f;

    Logger::Info(TeamcityTestsOutput::FormatTestFinished(testName, 
        ConverterUtils::NumberToString(minDelta),
        ConverterUtils::NumberToString(maxDelta), 
        ConverterUtils::NumberToString(averageDelta),
        ConverterUtils::NumberToString(testTime),
        ConverterUtils::NumberToString(elapsedTime)).c_str());
}

void BaseTest::SystemUpdate(float32 timeElapsed)
{
    bool frameForDebug = GetFrameNumber() > (GetDebugFrame() + BaseTest::FRAME_OFFSET);
    float32 delta = 0.0f;
    
    if (frameNumber > FRAME_OFFSET && !(IsDebuggable() && frameForDebug))
    {
        delta = fixedDelta > 0 ? fixedDelta : timeElapsed;

        frames.push_back(FrameInfo(timeElapsed, frameNumber));
        testTime += timeElapsed;

        PerformTestLogic(delta);
    }
    
    BaseScreen::SystemUpdate(delta);
}

void BaseTest::BeginFrame()
{
    if (frameNumber > (FRAME_OFFSET - 1) && 0 == startTime)
    {
        startTime = SystemTimer::Instance()->FrameStampTimeMS();
    }
}

void BaseTest::EndFrame()
{
    frameNumber++;
}