#include "stdafx.h"
#include "../../Renderer.h"
#include "UIElement.h"
#include "UISlider.h"
#include "../../../Input.h"

// -----------------------------------------------------------------------------
// CUISlider
// -----------------------------------------------------------------------------
CUISlider::CUISlider(
	int lX, int lY, int lWidth, int lHeight,
	float fMin, float fMax, float fCurrent, float fStep,
	ESliderOrientation eOrientation)
	: CUIElement(lX, lY, lWidth, lHeight)
	, m_fMin(fMin)
	, m_fMax(fMax)
	, m_fValue(fCurrent)
	, m_fStep(fStep)
	, m_eOrientation(eOrientation)
	, m_bDraggingThumb(false)
	, m_bHoverMinus(false)
	, m_bHoverPlus(false)
	, m_bHoverTrack(false)
	, m_dwBgColor(0x2A2A2AFF)
	, m_dwThumbColor(0x007ACCFF)
	, m_dwBtnNormalColor(0x404040FF)
	, m_dwBtnHoverColor(0x606060FF)
	, m_dwBtnPressedColor(0x1A5276FF)
	, m_dwTextColor(0xFFFFFFFF)
{
	if (m_fValue < m_fMin) m_fValue = m_fMin;
	if (m_fValue > m_fMax) m_fValue = m_fMax;
}

void CUISlider::SetValue(float fValue)
{
	float fOldValue = m_fValue;
	m_fValue = fValue;

	if (m_fValue < m_fMin) m_fValue = m_fMin;
	if (m_fValue > m_fMax) m_fValue = m_fMax;

	// Вызываем коллбек только если значение реально изменилось
	if (m_fValue != fOldValue && m_fnOnValueChanged)
	{
		m_fnOnValueChanged(m_fValue);
	}
}

void CUISlider::Update()
{
	CMouse* pMouse = CMouse::GetInstance();
	int mx = pMouse->GetX();
	int my = pMouse->GetY();

	int absX = GetAbsoluteX();
	int absY = GetAbsoluteY();

	m_bHovered = (mx >= absX && mx < absX + m_lWidth && my >= absY && my < absY + m_lHeight);

	// Вычисляем геометрию элементов
	bool bHoriz = (m_eOrientation == eSO_Horizontal);
	int btnSize = bHoriz ? m_lHeight : m_lWidth;

	int minusX = bHoriz ? absX : absX;
	int minusY = bHoriz ? absY : absY + m_lHeight - btnSize;

	int plusX = bHoriz ? absX + m_lWidth - btnSize : absX;
	int plusY = bHoriz ? absY : absY;

	int trackX = bHoriz ? absX + btnSize : absX;
	int trackY = bHoriz ? absY : absY + btnSize;
	int trackW = bHoriz ? m_lWidth - 2 * btnSize : m_lWidth;
	int trackH = bHoriz ? m_lHeight : m_lHeight - 2 * btnSize;

	auto IsInside = [&](int x, int y, int w, int h) {
		return (mx >= x && mx < x + w && my >= y && my < y + h);
		};

	m_bHoverMinus = IsInside(minusX, minusY, btnSize, btnSize);
	m_bHoverPlus = IsInside(plusX, plusY, btnSize, btnSize);
	m_bHoverTrack = IsInside(trackX, trackY, trackW, trackH);

	// 1. Логика перетаскивания (Drag) каретки
	if (m_bDraggingThumb)
	{
		if (pMouse->IsButtonDownRaw(CMouse::Button_Left))
		{
			float t = 0.0f;
			if (bHoriz)
			{
				t = static_cast<float>(mx - trackX) / static_cast<float>(trackW);
			}
			else
			{
				// Для вертикального: Максимум сверху, Минимум снизу
				t = 1.0f - (static_cast<float>(my - trackY) / static_cast<float>(trackH));
			}

			if (t < 0.0f) t = 0.0f;
			if (t > 1.0f) t = 1.0f;

			SetValue(m_fMin + t * (m_fMax - m_fMin));

			pMouse->ConsumeButton(CMouse::Button_Left);
		}
		else
		{
			m_bDraggingThumb = false; // Отпустили мышь
		}
	}
	else
	{
		// 2. Логика кликов (Кнопки + Трек)
		if (m_bHovered && pMouse->IsButtonPressed(CMouse::Button_Left))
		{
			if (m_bHoverMinus)
			{
				SetValue(m_fValue - m_fStep);
			}
			else if (m_bHoverPlus)
			{
				SetValue(m_fValue + m_fStep);
			}
			else if (m_bHoverTrack)
			{
				// Если кликнули по треку - сразу начинаем тащить и обновляем значение
				m_bDraggingThumb = true;

				float t = 0.0f;
				if (bHoriz) t = static_cast<float>(mx - trackX) / static_cast<float>(trackW);
				else        t = 1.0f - (static_cast<float>(my - trackY) / static_cast<float>(trackH));

				if (t < 0.0f) t = 0.0f;
				if (t > 1.0f) t = 1.0f;
				SetValue(m_fMin + t * (m_fMax - m_fMin));
			}

			pMouse->ConsumeButton(CMouse::Button_Left);
		}
	}
}

void CUISlider::Render(CRenderer* pRenderer)
{
	if (!m_bVisible || pRenderer == nullptr)
		return;

	int absX = GetAbsoluteX();
	int absY = GetAbsoluteY();
	bool bHoriz = (m_eOrientation == eSO_Horizontal);
	int btnSize = bHoriz ? m_lHeight : m_lWidth;

	// Расчет зон
	int minusX = bHoriz ? absX : absX;
	int minusY = bHoriz ? absY : absY + m_lHeight - btnSize;
	int plusX = bHoriz ? absX + m_lWidth - btnSize : absX;
	int plusY = bHoriz ? absY : absY;
	int trackX = bHoriz ? absX + btnSize : absX;
	int trackY = bHoriz ? absY : absY + btnSize;
	int trackW = bHoriz ? m_lWidth - 2 * btnSize : m_lWidth;
	int trackH = bHoriz ? m_lHeight : m_lHeight - 2 * btnSize;

	// 1. Отрисовка трека
	pRenderer->DrawRect(trackX, trackY, trackW, trackH, m_dwBgColor);

	// 2. Отрисовка кнопок
	CMouse* pMouse = CMouse::GetInstance();
	bool bMouseDown = pMouse->IsButtonDownRaw(CMouse::Button_Left);

	unsigned int clrMinus = m_bHoverMinus ? (bMouseDown ? m_dwBtnPressedColor : m_dwBtnHoverColor) : m_dwBtnNormalColor;
	unsigned int clrPlus = m_bHoverPlus ? (bMouseDown ? m_dwBtnPressedColor : m_dwBtnHoverColor) : m_dwBtnNormalColor;

	pRenderer->DrawRect(minusX, minusY, btnSize, btnSize, clrMinus);
	pRenderer->DrawRect(plusX, plusY, btnSize, btnSize, clrPlus);

	// Текст на кнопках (шрифт подгоняем под размер)
	pRenderer->DrawText(minusX + btnSize / 2, minusY + btnSize / 2 + 4, "-", m_dwTextColor, 14, TEXT_ALIGN_CENTER);
	pRenderer->DrawText(plusX + btnSize / 2, plusY + btnSize / 2 + 4, "+", m_dwTextColor, 14, TEXT_ALIGN_CENTER);

	// 3. Расчет и отрисовка каретки (Thumb)
	float t = (m_fValue - m_fMin) / (m_fMax - m_fMin);
	if (t < 0.0f) t = 0.0f;
	if (t > 1.0f) t = 1.0f;

	int thumbW = bHoriz ? 12 : trackW;
	int thumbH = bHoriz ? trackH : 12;

	int thumbX = bHoriz ? trackX + static_cast<int>(t * (trackW - thumbW)) : trackX;
	int thumbY = bHoriz ? trackY : trackY + static_cast<int>((1.0f - t) * (trackH - thumbH)); // Инверсия Y для верт. слайдера

	// Подсветка каретки, если тащим её
	unsigned int activeThumbColor = m_bDraggingThumb ? m_dwBtnPressedColor : m_dwThumbColor;
	pRenderer->DrawRect(thumbX, thumbY, thumbW, thumbH, activeThumbColor);

	// 4. Отрисовка текстовых значений (Min, Max, Value)
	if (bHoriz)
	{
		// Мин. / Макс. рисуем по краям трека
		pRenderer->DrawTextF(trackX + 4, trackY + trackH / 2 + 4, 0x888888FF, 10, TEXT_ALIGN_LEFT, "%.1f", m_fMin);
		pRenderer->DrawTextF(trackX + trackW - 4, trackY + trackH / 2 + 4, 0x888888FF, 10, TEXT_ALIGN_RIGHT, "%.1f", m_fMax);

		// Текущее значение - ровно по центру
		pRenderer->DrawTextF(trackX + trackW / 2, trackY + trackH / 2 + 4, m_dwTextColor, 12, TEXT_ALIGN_CENTER, "%.2f", m_fValue);
	}
	else
	{
		// Для вертикального: Макс сверху, Мин снизу
		pRenderer->DrawTextF(trackX + trackW / 2, trackY + 12, 0x888888FF, 10, TEXT_ALIGN_CENTER, "%.1f", m_fMax);
		pRenderer->DrawTextF(trackX + trackW / 2, trackY + trackH - 4, 0x888888FF, 10, TEXT_ALIGN_CENTER, "%.1f", m_fMin);
	}
}