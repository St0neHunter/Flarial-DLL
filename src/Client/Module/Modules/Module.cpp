#include "Module.hpp"
#include "../../Client.hpp"
#include "ClickGUI/ClickGUIRenderer.hpp"

std::map<std::string, DWRITE_TEXT_ALIGNMENT> alignments = {
        {"Left", DWRITE_TEXT_ALIGNMENT_LEADING},
        {"Center", DWRITE_TEXT_ALIGNMENT_CENTER},
        {"Right", DWRITE_TEXT_ALIGNMENT_TRAILING}
};

void Module::NormalRender(int index, std::string text, std::string value) {

    if(settings.getSettingByName<bool>("enabled")->value && SDK::CurrentScreen == "hud_screen") {
        DWRITE_TEXT_ALIGNMENT alignment = alignments[this->settings.getSettingByName<std::string>("textalignment")->value];
        bool responsivewidth = this->settings.getSettingByName<bool>("responsivewidth")->value;
        float paddingX = this->settings.getSettingByName<float>("padx")->value;
        float paddingY = this->settings.getSettingByName<float>("pady")->value;

        if (this->settings.getSettingByName<bool>("reversepaddingx")->value) paddingX = -(this->settings.getSettingByName<float>("padx")->value);
        if (this->settings.getSettingByName<bool>("reversepaddingy")->value) paddingY = -(this->settings.getSettingByName<float>("pady")->value);

        std::string uppercaseSentence;
        std::string search = "{VALUE}";

        for (char c : text) {
            uppercaseSentence += std::toupper(c);
        }

        size_t pos = uppercaseSentence.find(search);
        if (pos != std::string::npos) {
            text.replace(pos, search.length(), value);
        }

        float textWidth = Constraints::RelativeConstraint(0.7f * settings.getSettingByName<float>("uiscale")->value);
        float textHeight = Constraints::RelativeConstraint(0.1f * settings.getSettingByName<float>("uiscale")->value);

        float textSize = Constraints::SpacingConstraint(3.2f, textHeight) * this->settings.getSettingByName<float>("textscale")->value;

        DWRITE_TEXT_METRICS textMetrics;

        IDWriteTextFormat* textFormat;
        FlarialGUI::writeFactory->CreateTextFormat(FlarialGUI::to_wide(Client::settings.getSettingByName<std::string>("fontname")->value).c_str(), NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, Constraints::FontScaler(textSize), L"", &textFormat);
        textFormat->SetTextAlignment(alignment);
        textFormat->SetParagraphAlignment((DWRITE_PARAGRAPH_ALIGNMENT)alignment);

        IDWriteTextLayout* textLayout;

        FlarialGUI::writeFactory->CreateTextLayout(
                FlarialGUI::to_wide(text).c_str(),
                wcslen(FlarialGUI::to_wide(text).c_str()),
                textFormat,
                textWidth,
                textHeight,
                &textLayout
        );

        textLayout->GetMetrics(&textMetrics);
        textLayout->Release();
        textFormat->Release();

        if(!responsivewidth) textMetrics.left = 0;

        Vec2<float> settingperc = Vec2<float>(
                this->settings.getSettingByName<float>("percentageX")->value,
                this->settings.getSettingByName<float>("percentageY")->value
        );

        float realspacing = Constraints::SpacingConstraint(0.05f, textWidth);
        float rectWidth = (!responsivewidth
                ? (Constraints::RelativeConstraint(0.225f * settings.getSettingByName<float>("uiscale")->value)  * this->settings.getSettingByName<float>("rectwidth")->value)
                : (textMetrics.width + Constraints::SpacingConstraint(2.0, realspacing)) * this->settings.getSettingByName<float>("rectwidth")->value);

        Vec2<float> realcenter;

        if (settingperc.x != 0) realcenter = Vec2<float>(settingperc.x * MC::windowSize.x, settingperc.y * MC::windowSize.y);
        else realcenter = Constraints::CenterConstraint(rectWidth, textHeight * this->settings.getSettingByName<float>("rectheight")->value);

        if (ModuleManager::getModule("ClickGUI")->settings.getSettingByName<bool>("enabled")->value || ClickGUIRenderer::editmenu) {
            FlarialGUI::SetWindowRect(realcenter.x + textMetrics.left, realcenter.y, rectWidth, textHeight * this->settings.getSettingByName<float>("rectheight")->value, index, -textMetrics.left);

            Vec2<float> vec2 = FlarialGUI::CalculateMovedXY(realcenter.x, realcenter.y, index, rectWidth, textHeight * this->settings.getSettingByName<float>("rectheight")->value);

            realcenter.x = vec2.x;
            realcenter.y = vec2.y;

            realcenter = realcenter;

            Vec2<float> percentages = Constraints::CalculatePercentage(realcenter.x, realcenter.y);

            this->settings.setValue("percentageX", percentages.x);
            this->settings.setValue("percentageY", percentages.y);
        }

        Vec2<float> rounde = Constraints::RoundingConstraint(this->settings.getSettingByName<float>("rounding")->value * settings.getSettingByName<float>("uiscale")->value, this->settings.getSettingByName<float>("rounding")->value * settings.getSettingByName<float>("uiscale")->value);

        D2D1_COLOR_F bgColor = FlarialGUI::HexToColorF(settings.getSettingByName<std::string>("bgColor")->value);
        D2D1_COLOR_F textColor = FlarialGUI::HexToColorF(settings.getSettingByName<std::string>("textColor")->value);
        D2D1_COLOR_F borderColor = FlarialGUI::HexToColorF(settings.getSettingByName<std::string>("borderColor")->value);

        bgColor.a = settings.getSettingByName<float>("bgOpacity")->value;
        textColor.a = settings.getSettingByName<float>("textOpacity")->value;
        borderColor.a = settings.getSettingByName<float>("borderOpacity")->value;


        if (settings.getSettingByName<bool>("BlurEffect")->value) FlarialGUI::BlurRect(D2D1::RoundedRect(D2D1::RectF(realcenter.x + textMetrics.left, realcenter.y, realcenter.x + rectWidth + textMetrics.left, realcenter.y + (textHeight) * this->settings.getSettingByName<float>("rectheight")->value), rounde.x, rounde.x), Client::settings.getSettingByName<float>("blurintensity")->value);

        FlarialGUI::RoundedRect(
                realcenter.x + textMetrics.left,
                realcenter.y,
                bgColor,
                rectWidth,
                textHeight * this->settings.getSettingByName<float>("rectheight")->value,
                rounde.x,
                rounde.x
        );

        FlarialGUI::FlarialTextWithFont(
                realcenter.x + Constraints::SpacingConstraint(paddingX, textWidth) + textMetrics.left,
                realcenter.y + Constraints::SpacingConstraint(paddingY, textWidth),
                FlarialGUI::to_wide(text).c_str(),
                textColor,
                rectWidth,
                textHeight,
                alignment,
                textSize
        );

        if (this->settings.getSettingByName<bool>("border")->value) {
            FlarialGUI::RoundedHollowRect(
                    realcenter.x + textMetrics.left,
                    realcenter.y,
                    Constraints::RelativeConstraint((this->settings.getSettingByName<float>("borderWidth")->value * settings.getSettingByName<float>("uiscale")->value) / 100.0f, "height", true),
                    borderColor,
                    rectWidth,
                    textHeight * this->settings.getSettingByName<float>("rectheight")->value,
                    rounde.x,
                    rounde.x
            );
        }

        if (ModuleManager::getModule("ClickGUI")->settings.getSettingByName<bool>("enabled")->value || ClickGUIRenderer::editmenu)
            FlarialGUI::UnsetWindowRect();
    }
}