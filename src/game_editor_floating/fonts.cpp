//
// Created by mgrus on 15.04.2025.
//

#define UNICODE
#include <string>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite_3.h>
#include <wincodec.h>

static IDWriteFactory5* m_pDWriteFactory;


void createDWriteFont(const std::wstring& fontName, IDWriteTextFormat** targetTextFormat, float size) {

    HRESULT hr = S_OK;

    if (!m_pDWriteFactory) {
        // Create a DirectWrite factory.
        HRESULT hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory5),
            reinterpret_cast<IUnknown **>(&m_pDWriteFactory)
            );
    }

    if (SUCCEEDED(hr))
    {

        // Load our system fonts and build corresponding textFormat objects
        IDWriteFontFile* fontFile;
        m_pDWriteFactory->CreateFontFileReference(fontName.c_str(), nullptr, &fontFile);

        IDWriteFontSet* fontSet;
        IDWriteFontCollection1* fontCollection;
        IDWriteFontSetBuilder1* fontSetBuilder;
        m_pDWriteFactory->CreateFontSetBuilder(&fontSetBuilder);
        fontSetBuilder->AddFontFile(fontFile);
        fontSetBuilder->CreateFontSet(&fontSet);
        m_pDWriteFactory->CreateFontCollectionFromFontSet(fontSet, &fontCollection);

        hr = m_pDWriteFactory->CreateTextFormat(
            L"Orbitron",
            fontCollection,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            size,
            L"en-us", //locale
            targetTextFormat
            );
    }
    if (SUCCEEDED(hr))
    {
        // Center the text horizontally and vertically.
        (*targetTextFormat)->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        (*targetTextFormat)->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    }
}
