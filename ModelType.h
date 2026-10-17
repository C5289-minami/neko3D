#pragma once

// シェーダー用にモデルの種類を定義する列挙型

enum class ModelType
{
	Model = 0, // ボーン付きモデル
	Bone = 1, // ボーンなしモデル
};