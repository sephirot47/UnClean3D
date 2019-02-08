#include "GLSLArrayOfArrays.h"

#include "Bang/Assets.h"
#include "Bang/ShaderProgram.h"

using namespace Bang;

GLSLArrayOfArrays::GLSLArrayOfArrays()
{
    m_arrayTexture = Assets::Create<Texture2D>();
    GetArrayTexture()->CreateEmpty(Vector2i(TexSize));
    GetArrayTexture()->SetFilterMode(GL::FilterMode::NEAREST);
}

GLSLArrayOfArrays::~GLSLArrayOfArrays()
{
}

void GLSLArrayOfArrays::SetFormat(GL::ColorFormat colorFormat)
{
    GetArrayTexture()->SetFormat(colorFormat);
}

void GLSLArrayOfArrays::Fill(const Array<Array<Vector4>> &array)
{
    const uint TotalTexSize = (TexSize * TexSize);
    Array<Vector4> arrayTextureData(TotalTexSize);

    // First fill begin/end indices of each array.
    const uint innerArrayDataBeginning = array.Size();

    int totalInnerArraysSize = 0;
    int currentTexCoord = 0;
    for (const Array<Vector4> &innerArray : array)
    {
        const uint innerArraySize = innerArray.Size();
        const uint innerArrayDataBeginCoord =
            innerArrayDataBeginning + totalInnerArraysSize;

        arrayTextureData[currentTexCoord][0] = innerArrayDataBeginCoord;
        arrayTextureData[currentTexCoord][1] =
            innerArrayDataBeginCoord + innerArraySize;

        ++currentTexCoord;
        totalInnerArraysSize += innerArraySize;
    }
    m_numOuterArrays = array.Size();

    for (const Array<Vector4> &innerArray : array)
    {
        for (const Vector4 &element : innerArray)
        {
            if (currentTexCoord < TotalTexSize)
            {
                arrayTextureData[currentTexCoord] = element;
                ++currentTexCoord;
            }
            else
            {
                Debug_Error("Not enough texture size to represent the "
                            "uniform grid");
                ASSERT(false);
                exit(1);
            }
        }
    }

    GetArrayTexture()->Fill(RCAST<Byte *>(arrayTextureData.Data()),
                            TexSize,
                            TexSize,
                            GL::ColorComp::RGBA,
                            GL::DataType::FLOAT);
}

Texture2D *GLSLArrayOfArrays::GetArrayTexture() const
{
    return m_arrayTexture.Get();
}

void GLSLArrayOfArrays::Bind(const String &arrayName, ShaderProgram *sp)
{
    sp->SetTexture2D(arrayName, GetArrayTexture());
    sp->SetInt(arrayName + "_HeaderSize", m_numOuterArrays);
    sp->SetVector2(arrayName + "_Size", Vector2(TexSize));
    sp->SetVector2(arrayName + "_TexelUvSize", Vector2(1.0f / TexSize));
}
