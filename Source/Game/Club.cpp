#include "Club.h"

const ClubData Club::CLUBS[] =
{
    { "Driver", 55.0f, 14.0f, { 0.90f, 0.30f, 0.20f, 1.0f } },
    { "Iron",   35.0f, 28.0f, { 0.30f, 0.50f, 0.90f, 1.0f } },
    { "Putter",  8.0f,  0.0f, { 0.30f, 0.85f, 0.30f, 1.0f } },
};

int Club::GetCount()
{
    return static_cast<int>(sizeof(CLUBS) / sizeof(CLUBS[0]));
}

void Club::Next()
{
    m_index = (m_index + 1) % GetCount();
}

void Club::Prev()
{
    m_index = (m_index + GetCount() - 1) % GetCount();
}