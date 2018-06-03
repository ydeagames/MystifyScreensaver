//__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/
//! @file   GameMain.cpp
//!
//! @brief  Mystifyスクリーンセーバー
//!
//! @date   2018/05/30
//!
//! @author GF1 26 山口寛雅
//__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/__/

// ヘッダファイルの読み込み ================================================
#include "GameMain.h"
#include <deque>
#include <stdlib.h>
#include <cmath>

// 定数の宣言 ==============================================================
#define VERTEX_COMPOUNDS 2
#define VERTEX_SQUARES_MIN 7
#define VERTEX_SQUARES_MAX 15
#define VERTEX_VERTEX 4
#define VERTEX_INTERVAL 2
#define VERTEX_VEL_MIN 1
#define VERTEX_VEL_MAX 6

// 構造体の宣言 ============================================================
struct MovingVertex {
	float pos_x, pos_y;
	float vel_x, vel_y;
};

struct MovingSquare {
	std::deque<MovingVertex> vertex;
};

struct MovingSquareCompound {
	float color_h;
	MovingSquare latest;
	size_t history_size;
	std::deque<MovingSquare> history;
};

// グローバル変数の定義 ====================================================
std::deque<MovingSquareCompound> compounds;



// 関数の定義 ==============================================================

int irand(int min, int max)
{
	return rand() % (max - min + 1) + min;
}

float irandf(int min, int max, int presision)
{
	return (rand() % ((max - min + 1)*presision)) / (float)presision + min;
}

// H : 色相　有効範囲 0.0f ～ 360.0f
// S : 彩度　有効範囲 0.0f ～ 1.0f
// V : 明度　有効範囲 0.0f ～ 1.0f
int GetColorHSV(float H, float S, float V)
{
	int hi;
	float f, p, q, t;
	float r, g, b;
	int ir, ig, ib;

	hi = (int)(H / 60.0f);
	hi = hi == 6 ? 5 : hi %= 6;
	f = H / 60.0f - (float)hi;
	p = V * (1.0f - S);
	q = V * (1.0f - f * S);
	t = V * (1.0f - (1.0f - f) * S);
	switch (hi)
	{
	case 0: r = V; g = t; b = p; break;
	case 1: r = q; g = V; b = p; break;
	case 2: r = p; g = V; b = t; break;
	case 3: r = p; g = q; b = V; break;
	case 4: r = t; g = p; b = V; break;
	case 5: r = V; g = p; b = q; break;
	}

	ir = (int)(r * 255.0f);
	if (ir > 255) ir = 255;
	else if (ir < 0) ir = 0;

	ig = (int)(g * 255.0f);
	if (ig > 255) ig = 255;
	else if (ig < 0) ig = 0;

	ib = (int)(b * 255.0f);
	if (ib > 255) ib = 255;
	else if (ib < 0) ib = 0;

	return GetColor(ir, ig, ib);
}

void MoveVertex(MovingVertex &vertex)
{
	vertex.pos_x += vertex.vel_x;
	vertex.pos_y += vertex.vel_y;

	if (vertex.pos_x < SCREEN_LEFT || SCREEN_RIGHT <= vertex.pos_x)
	{
		float speed = irandf(VERTEX_VEL_MIN, VERTEX_VEL_MAX, 100);
		vertex.vel_x = (vertex.vel_x > 0) ? -speed : speed;

		if (vertex.pos_x < SCREEN_LEFT)
			vertex.pos_x = SCREEN_LEFT; // 壁にめり込まないように修正
		else
			vertex.pos_x = SCREEN_RIGHT - 1.f;
	}
	if (vertex.pos_y < SCREEN_TOP || SCREEN_BOTTOM <= vertex.pos_y)
	{
		float speed = irandf(VERTEX_VEL_MIN, VERTEX_VEL_MAX, 100);
		vertex.vel_y = (vertex.vel_y > 0) ? -speed : speed;

		if (vertex.pos_y < SCREEN_TOP)
			vertex.pos_y = SCREEN_TOP; // 壁にめり込まないように修正
		else
			vertex.pos_y = SCREEN_BOTTOM - 1.f;
	}
}

void MoveSquare(MovingSquare &square)
{
	for (auto itr = square.vertex.begin(); itr != square.vertex.end(); ++itr)
	{
		MoveVertex(*itr);
	}
}

void MoveSquareCompound(MovingSquareCompound &compound)
{
	compound.color_h = fmodf((compound.color_h + .1f), 360.f);

	MoveSquare(compound.latest);
	compound.history.push_back(compound.latest);
	if (compound.history.size() > compound.history_size * VERTEX_INTERVAL)
		compound.history.pop_front();
}

//----------------------------------------------------------------------
//! @brief ゲームの初期化処理
//!
//! @param[in] なし
//!
//! @return なし
//----------------------------------------------------------------------
void InitializeGame(void)
{
	for (int n = 0; n < VERTEX_COMPOUNDS; n++)
	{
		MovingSquareCompound compound;
		compound.color_h = (rand() % 360 * 100) / 100.f;
		compound.history_size = irand(VERTEX_SQUARES_MIN, VERTEX_SQUARES_MAX);

		{
			MovingSquare square;

			for (int ix = 0; ix < VERTEX_VERTEX; ix++)
			{
				struct MovingVertex vertex = {
					(float)(rand() % SCREEN_WIDTH),
					(float)(rand() % SCREEN_HEIGHT),
					irandf(VERTEX_VEL_MIN, VERTEX_VEL_MAX, 100),
					irandf(VERTEX_VEL_MIN, VERTEX_VEL_MAX, 100),
				};
				square.vertex.push_back(vertex);
			}

			compound.latest = square;
		}

		compounds.push_back(compound);
	}
}



//----------------------------------------------------------------------
//! @brief ゲームの更新処理
//!
//! @param[in] なし
//!
//! @return なし
//----------------------------------------------------------------------
void UpdateGame(void)
{
	for (auto itr = compounds.begin(); itr != compounds.end(); ++itr)
	{
		MoveSquareCompound(*itr);
	}
}



//----------------------------------------------------------------------
//! @brief ゲームの描画処理
//!
//! @param[in] なし
//!
//! @return なし
//----------------------------------------------------------------------
void RenderGame(void)
{
	for (auto it = compounds.begin(); it != compounds.end(); ++it)
	{
		MovingSquareCompound &compound = *it;

		int i = 0;
		// OKな例
		// for (int i = 0, j = 20; i < 16 && 16 < j; i++, j--)
		// ダメな例
		// for (auto iter = compound.history.begin(), i = 0; iter != compound.history.end(); ++iter, i++)
		for (auto iter = compound.history.begin(); iter != compound.history.end(); ++iter, i++)
		{
			if (i%VERTEX_INTERVAL == 0)
			{
				MovingSquare &square = *iter;

				{
					MovingVertex *last_vertex = &square.vertex.front();

					for (auto itr = square.vertex.rbegin(), e = square.vertex.rend(); itr != e; ++itr)
					{
						MovingVertex &vertex = *itr;

						DrawLineAA(
							last_vertex->pos_x,
							last_vertex->pos_y,
							vertex.pos_x,
							vertex.pos_y,
							GetColorHSV(compound.color_h, 1.f, 1.f),
							1.f
						);

						last_vertex = &vertex;
					}
				}
			}
		}
	}
}



//----------------------------------------------------------------------
//! @brief ゲームの終了処理
//!
//! @param[in] なし
//!
//! @return なし
//----------------------------------------------------------------------
void FinalizeGame(void)
{

}
