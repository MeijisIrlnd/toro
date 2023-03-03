//
// Created by Syl on 23/02/2023.
//
#pragma once

namespace Toro
{
    class SinglePoleLowpass
    {
    public:

        void setCoeff(const float newCoeff) noexcept {
            m_coeff = newCoeff;
        }

        float processSample(float in) {
            in *= 1 - m_coeff;
            auto current = in + m_x1;
            m_x1 = current * m_coeff;
            return current;
        }


    private:
        float m_x1{ 0.0f };
        float m_coeff{ 0.9995f };
    };
}