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



#include "UI/UIControlBackground.h"
#include "Debug/DVAssert.h"
#include "UI/UIControl.h"
#include "Core/Core.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"
#include "Render/2D/RenderSystem2D/VirtualCoordinatesSystem.h"
#include "Render/2D/RenderSystem2D/RenderSystem2D.h"

#include <limits>

namespace DAVA
{

const uint16 UIControlBackground::StretchDrawData::indeces[18 * 3] = {
    0, 1, 4,
    1, 5, 4,
    1, 2, 5,
    2, 6, 5,
    2, 3, 6,
    3, 7, 6,
            
    4, 5, 8,
    5, 9, 8,
    5, 6, 9,
    6, 10, 9,
    6, 7, 10,
    7, 11, 10,
            
    8, 9, 12,
    9, 12, 13,
    9, 10, 13,
    10, 14, 13,
    10, 11, 14,
    11, 15, 14
};

UIControlBackground::UIControlBackground()
:	spr(NULL)
,	frame(0)
,	align(ALIGN_HCENTER|ALIGN_VCENTER)
,	type(DRAW_ALIGNED)
,	color(Color::White)
,	drawColor(Color::White)
,	leftStretchCap(0)
,	topStretchCap(0)
,	spriteModification(0)
,	colorInheritType(COLOR_IGNORE_PARENT)
,	perPixelAccuracyType(PER_PIXEL_ACCURACY_DISABLED)
,	lastDrawPos(0, 0)
,	tiledData(NULL)
,   stretchData(NULL)
,	shader(SafeRetain(RenderManager::TEXTURE_MUL_FLAT_COLOR))
,   margins(NULL)
,   renderState(RenderState::RENDERSTATE_2D_BLEND)
{
}

UIControlBackground *UIControlBackground::Clone()
{
    UIControlBackground *cb = new UIControlBackground();
    cb->CopyDataFrom(this);
    return cb;
}

void UIControlBackground::CopyDataFrom(UIControlBackground *srcBackground)
{
    SafeRelease(spr);
    spr = SafeRetain(srcBackground->spr);
    frame = srcBackground->frame;
    align = srcBackground->align;

    SetDrawType(srcBackground->type);
    SetMargins(srcBackground->GetMargins());

    color = srcBackground->color;
    spriteModification = srcBackground->spriteModification;
    colorInheritType = srcBackground->colorInheritType;
    perPixelAccuracyType = srcBackground->perPixelAccuracyType;
    leftStretchCap = srcBackground->leftStretchCap;
    topStretchCap = srcBackground->topStretchCap;

    SetShader(srcBackground->shader);
}


UIControlBackground::~UIControlBackground()
{
    SafeRelease(spr);
    SafeRelease(shader);
    SafeDelete(margins);
    ReleaseDrawData();
}

bool UIControlBackground::IsEqualTo( const UIControlBackground *back ) const
{
    if (GetDrawType() != back->GetDrawType() ||
        Sprite::GetPathString(GetSprite()) != Sprite::GetPathString(back->GetSprite()) ||
        GetFrame() != back->GetFrame() ||
        GetAlign() != back->GetAlign() ||
        GetColor() != back->GetColor() ||
        GetColorInheritType() != back->GetColorInheritType() ||
        GetModification() != back->GetModification() ||
        GetLeftRightStretchCap() != back->GetLeftRightStretchCap() ||
        GetTopBottomStretchCap() != back->GetTopBottomStretchCap() ||
        GetPerPixelAccuracyType() != back->GetPerPixelAccuracyType() ||
        GetMargins() != back->GetMargins())
        return false;
    return true;
}

Sprite*	UIControlBackground::GetSprite() const
{
    return spr;
}
int32	UIControlBackground::GetFrame() const
{
    return frame;
}
int32	UIControlBackground::GetAlign() const
{
    return align;
}

int32	UIControlBackground::GetModification() const
{
    return spriteModification;
}

UIControlBackground::eColorInheritType UIControlBackground::GetColorInheritType() const
{
    return (eColorInheritType)colorInheritType;
}


UIControlBackground::eDrawType	UIControlBackground::GetDrawType() const
{
    return type;
}


void UIControlBackground::SetSprite(const FilePath &spriteName, int32 drawFrame)
{
    Sprite *tempSpr = Sprite::Create(spriteName);
    SetSprite(tempSpr, drawFrame);
    SafeRelease(tempSpr);
}

void UIControlBackground::SetSprite(Sprite* drawSprite, int32 drawFrame)
{
    if (drawSprite == this->spr)
    {
        // Sprite is not changed - update frame only.
        frame = drawFrame;
        return;
    }

    SafeRelease(spr);
    spr = SafeRetain(drawSprite);
    frame =  drawFrame;
}
void UIControlBackground::SetFrame(int32 drawFrame)
{
    DVASSERT(spr);
    frame = drawFrame;
}

void UIControlBackground::SetAlign(int32 drawAlign)
{
    align = drawAlign;
}
void UIControlBackground::SetDrawType(UIControlBackground::eDrawType drawType)
{
    type = drawType;
    ReleaseDrawData();
}

void UIControlBackground::SetModification(int32 modification)
{
    spriteModification = modification;
}

void UIControlBackground::SetColorInheritType(UIControlBackground::eColorInheritType inheritType)
{
    DVASSERT(inheritType >= 0 && inheritType < COLOR_INHERIT_TYPES_COUNT);
    colorInheritType = inheritType;
}

void UIControlBackground::SetPerPixelAccuracyType(ePerPixelAccuracyType accuracyType)
{
    perPixelAccuracyType = accuracyType;
}

UIControlBackground::ePerPixelAccuracyType UIControlBackground::GetPerPixelAccuracyType() const
{
    return perPixelAccuracyType;
}

const Color &UIControlBackground::GetDrawColor() const
{
    return drawColor;
}

void UIControlBackground::SetDrawColor(const Color &c)
{
    drawColor = c;
}

void UIControlBackground::SetParentColor(const Color &parentColor)
{
    switch (colorInheritType)
    {
        case COLOR_MULTIPLY_ON_PARENT:
        {
            drawColor.r = color.r * parentColor.r;
            drawColor.g = color.g * parentColor.g;
            drawColor.b = color.b * parentColor.b;
            drawColor.a = color.a * parentColor.a;
        }
            break;
        case COLOR_ADD_TO_PARENT:
        {
            drawColor.r = Min(color.r + parentColor.r, 1.0f);
            drawColor.g = Min(color.g + parentColor.g, 1.0f);
            drawColor.b = Min(color.b + parentColor.b, 1.0f);
            drawColor.a = Min(color.a + parentColor.a, 1.0f);
        }
            break;
        case COLOR_REPLACE_TO_PARENT:
        {
            drawColor = parentColor;
        }
            break;
        case COLOR_IGNORE_PARENT:
        {
            drawColor = color;
        }
            break;
        case COLOR_MULTIPLY_ALPHA_ONLY:
        {
            drawColor = color;
            drawColor.a = color.a * parentColor.a;
        }
            break;
        case COLOR_REPLACE_ALPHA_ONLY:
        {
            drawColor = color;
            drawColor.a = parentColor.a;
        }
            break;
    }
}

void UIControlBackground::Draw(const UIGeometricData &parentGeometricData)
{
    UIGeometricData geometricData;
    geometricData.size = parentGeometricData.size;
    if (margins)
    {
        geometricData.position = Vector2(margins->left, margins->top);
        geometricData.size += Vector2(-(margins->right + margins->left), -(margins->bottom + margins->top));
    }

    geometricData.AddToGeometricData(parentGeometricData);
    Rect drawRect = geometricData.GetUnrotatedRect();

    RenderManager::Instance()->SetColor(drawColor.r, drawColor.g, drawColor.b, drawColor.a);

    Sprite::DrawState drawState;
    drawState.SetRenderState(renderState);
    if (spr)
    {
        drawState.SetShader(shader);
        drawState.frame = frame;
        if (spriteModification)
        {
            drawState.flags = spriteModification;
        }
//		spr->Reset();
//		spr->SetFrame(frame);
//		spr->SetModification(spriteModification);
    }
    switch (type)
    {
        case DRAW_ALIGNED:
        {
            if (!spr)break;
            if(align & ALIGN_LEFT)
            {
                drawState.position.x = drawRect.x;
            }
            else if(align & ALIGN_RIGHT)
            {
                drawState.position.x = drawRect.x + drawRect.dx - spr->GetWidth() * geometricData.scale.x;
            }
            else
            {
                drawState.position.x = drawRect.x + ((drawRect.dx - spr->GetWidth() * geometricData.scale.x) * 0.5f) ;
            }
            if(align & ALIGN_TOP)
            {
                drawState.position.y = drawRect.y;
            }
            else if(align & ALIGN_BOTTOM)
            {
                drawState.position.y = drawRect.y + drawRect.dy - spr->GetHeight() * geometricData.scale.y;
            }
            else
            {
                drawState.position.y = drawRect.y + ((drawRect.dy - spr->GetHeight() * geometricData.scale.y + spr->GetDefaultPivotPoint().y * geometricData.scale.y) * 0.5f) ;
            }
            if(geometricData.angle != 0)
            {
                float tmpX = drawState.position.x;
                drawState.position.x = (tmpX - geometricData.position.x) * geometricData.cosA  + (geometricData.position.y - drawState.position.y) * geometricData.sinA + geometricData.position.x;
                drawState.position.y = (tmpX - geometricData.position.x) * geometricData.sinA  + (drawState.position.y - geometricData.position.y) * geometricData.cosA + geometricData.position.y;
//				spr->SetAngle(geometricData.angle);
                drawState.angle = geometricData.angle;
            }
//			spr->SetPosition(x, y);
            drawState.scale = geometricData.scale;
            drawState.pivotPoint = spr->GetDefaultPivotPoint();
//			spr->SetScale(geometricData.scale);
            //if (drawState.scale.x == 1.0 && drawState.scale.y == 1.0)
            {
                switch(perPixelAccuracyType)
                {
                    case PER_PIXEL_ACCURACY_ENABLED:
                        if(lastDrawPos == drawState.position)
                        {
                            drawState.usePerPixelAccuracy = true;
                        }
                        break;
                    case PER_PIXEL_ACCURACY_FORCED:
                        drawState.usePerPixelAccuracy = true;
                        break;
                    default:
                        break;
                }
            }

            lastDrawPos = drawState.position;
            RenderSystem2D::Instance()->Draw(spr, &drawState);
        }
        break;

        case DRAW_SCALE_TO_RECT:
        {
            if (!spr)break;

            drawState.position = geometricData.position;
            drawState.flags = spriteModification;
            drawState.scale.x = drawRect.dx / spr->GetSize().dx;
            drawState.scale.y = drawRect.dy / spr->GetSize().dy;
            drawState.pivotPoint.x = geometricData.pivotPoint.x / (geometricData.size.x / spr->GetSize().dx);
            drawState.pivotPoint.y = geometricData.pivotPoint.y / (geometricData.size.y / spr->GetSize().dy);
            drawState.angle = geometricData.angle;
            {
                switch(perPixelAccuracyType)
                {
                case PER_PIXEL_ACCURACY_ENABLED:
                    if(lastDrawPos == drawState.position)
                    {
                        drawState.usePerPixelAccuracy = true;
                    }
                    break;
                case PER_PIXEL_ACCURACY_FORCED:
                    drawState.usePerPixelAccuracy = true;
                    break;
                default:
                    break;
                }
            }

            lastDrawPos = drawState.position;

//			spr->SetPosition(geometricData.position);
//			spr->SetScale(drawRect.dx / spr->GetSize().dx, drawRect.dy / spr->GetSize().dy);
//			spr->SetPivotPoint(geometricData.pivotPoint.x / (geometricData.size.x / spr->GetSize().dx), geometricData.pivotPoint.y / (geometricData.size.y / spr->GetSize().dy));
//			spr->SetAngle(geometricData.angle);
            
            RenderSystem2D::Instance()->Draw(spr, &drawState);
        }
        break;

        case DRAW_SCALE_PROPORTIONAL:
        case DRAW_SCALE_PROPORTIONAL_ONE:
        {
            if (!spr)break;
            float32 w, h;
            w = drawRect.dx / (spr->GetWidth() * geometricData.scale.x);
            h = drawRect.dy / (spr->GetHeight() * geometricData.scale.y);
            float ph = spr->GetDefaultPivotPoint().y;

            if(w < h)
            {
                if(type==DRAW_SCALE_PROPORTIONAL_ONE)
                {
                    w = spr->GetWidth() * h * geometricData.scale.y;
                    ph *= h;
                    h = drawRect.dy;
                }
                else
                {
                    h = spr->GetHeight() * w * geometricData.scale.x;
                    ph *= w;
                    w = drawRect.dx;
                }
            }
            else
            {
                if(type==DRAW_SCALE_PROPORTIONAL_ONE)
                {
                    h = spr->GetHeight() * w * geometricData.scale.x;
                    ph *= w;
                    w = drawRect.dx;
                }
                else
                {
                    w = spr->GetWidth() * h * geometricData.scale.y;
                    ph *= h;
                    h = drawRect.dy;
                }
            }

            if(align & ALIGN_LEFT)
            {
                drawState.position.x = drawRect.x;
            }
            else if(align & ALIGN_RIGHT)
            {
                drawState.position.x = (drawRect.x + drawRect.dx - w);
            }
            else
            {
                drawState.position.x = drawRect.x + (int32)((drawRect.dx - w) * 0.5) ;
            }
            if(align & ALIGN_TOP)
            {
                drawState.position.y = drawRect.y;
            }
            else if(align & ALIGN_BOTTOM)
            {
                drawState.position.y = (drawRect.y + drawRect.dy - h);
            }
            else
            {
                drawState.position.y = (drawRect.y) + (int32)((drawRect.dy - h + ph) * 0.5) ;
            }
            drawState.scale.x = w / spr->GetWidth();
            drawState.scale.y = h / spr->GetHeight();
//			spr->SetScaleSize(w, h);
            if(geometricData.angle != 0)
            {
                float32 tmpX = drawState.position.x;
                drawState.position.x = ((tmpX - geometricData.position.x) * geometricData.cosA  + (geometricData.position.y - drawState.position.y) * geometricData.sinA + geometricData.position.x);
                drawState.position.y = ((tmpX - geometricData.position.x) * geometricData.sinA  + (drawState.position.y - geometricData.position.y) * geometricData.cosA + geometricData.position.y);
                drawState.angle = geometricData.angle;
//				spr->SetAngle(geometricData.angle);
            }
//			spr->SetPosition((float32)x, (float32)y);
            {
                switch(perPixelAccuracyType)
                {
                case PER_PIXEL_ACCURACY_ENABLED:
                    if(lastDrawPos == drawState.position)
                    {
                        drawState.usePerPixelAccuracy = true;
                    }
                    break;
                case PER_PIXEL_ACCURACY_FORCED:
                    drawState.usePerPixelAccuracy = true;
                    break;
                default:
                    break;
                }
            }

            lastDrawPos = drawState.position;
            
            RenderSystem2D::Instance()->Draw(spr, &drawState);
        }
        break;

        case DRAW_FILL:
        	RenderSystem2D::Instance()->DrawFilled(spr, &drawState, geometricData);
        break;

        case DRAW_STRETCH_BOTH:
        case DRAW_STRETCH_HORIZONTAL:
        case DRAW_STRETCH_VERTICAL:
            RenderSystem2D::Instance()->DrawStretched(spr, &drawState, Vector2(leftStretchCap, topStretchCap), drawRect, type);
        break;

        case DRAW_TILED:
            RenderSystem2D::Instance()->DrawTiled(spr, &drawState, Vector2(leftStretchCap, topStretchCap), geometricData, &tiledData);
        break;
        default:
            break;
    }

    RenderManager::Instance()->ResetColor();

}

void UIControlBackground::ReleaseDrawData()
{
    SafeDelete(tiledData);
    SafeDelete(stretchData);
}

void UIControlBackground::SetLeftRightStretchCap(float32 _leftStretchCap)
{
    leftStretchCap = _leftStretchCap;
}

void UIControlBackground::SetTopBottomStretchCap(float32 _topStretchCap)
{
    topStretchCap = _topStretchCap;
}

float32 UIControlBackground::GetLeftRightStretchCap() const
{
    return leftStretchCap;
}

float32 UIControlBackground::GetTopBottomStretchCap() const
{
    return topStretchCap;
}

void UIControlBackground::SetShader(Shader *_shader)
{
    if(shader != _shader)
    {
        SafeRelease(shader);
        shader = SafeRetain(_shader);
    }
}

void UIControlBackground::SetMargins(const UIMargins* uiMargins)
{
    if (!uiMargins || uiMargins->empty())
    {
        SafeDelete(margins);
        return;
    }

    if (!margins)
    {
        margins = new UIControlBackground::UIMargins();
    }

    *margins = *uiMargins;
}

};
