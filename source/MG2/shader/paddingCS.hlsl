// =======================================================
// Padding Removal Compute Shader
// ・分割データ内のパディング領域を詰める
// ・ByteAddressBuffer単位でコピーすることで汎用化
// ・divisionごとに、padding分だけ前方へ詰めて書き込む
// =======================================================

#include "common.hlsl"

// 出力（パディング除去後のバッファ）
RWByteAddressBuffer Result : register(u0);

// 各divisionのメタ情報
// offset  : 元データ開始位置
// count   : 要素数
// padding : 前方に詰める量
StructuredBuffer<BOOKMARK> DivisionBookmarks : register(t0);

// 元データ（パディングあり）
ByteAddressBuffer Original : register(t1);

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    // Y方向：divisionIndex（どの塊を処理するか）
    uint divisionIndex = DTid.y;

    // X方向：要素内のオフセット（4byte単位）
    uint addrOffset = DTid.x;

    // 範囲外スレッドは終了
    if (divisionIndex >= CSMaxY)
        return;
    if (addrOffset >= CSMaxX)
        return;

    // 1要素あたりのstride（4byte単位）
    // ※CSMaxXは「1要素のサイズ / 4byte」
    uint stride = CSMaxX;

    // divisionメタ取得
    BOOKMARK meta = DivisionBookmarks[divisionIndex];

    uint begin = meta.offset; // 元データ開始インデックス
    uint count = meta.count; // 要素数
    uint padding = meta.padding; // 詰める量（削除する先頭領域）

    // =======================================================
    // 各要素を順にコピー（padding分だけ前詰め）
    // =======================================================
    for (uint i = 0; i < count; ++i)
    {
        // 元の要素Index
        uint srcIndex = begin + i;

        // パディングを引いた書き込み先Index
        uint dstIndex = srcIndex - padding;

        // ByteAddressBuffer用のバイトアドレスへ変換
        // strideは4byte単位なので最後に *4
        uint srcByteAddr = (srcIndex * stride + addrOffset) * 4;
        uint dstByteAddr = (dstIndex * stride + addrOffset) * 4;

        // 4byte単位でコピー
        uint v = Original.Load(srcByteAddr);
        Result.Store(dstByteAddr, v);
    }
}