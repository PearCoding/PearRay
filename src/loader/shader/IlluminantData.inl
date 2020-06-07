// All data is based on CIE 15: Technical Report: Colorimetry, 3rd edition
// Renormalized by 100 for all data (F also!)

constexpr size_t CIE_SampleCount	= 107;
constexpr float CIE_WavelengthStart = 300.0f;
constexpr float CIE_WavelengthEnd	= 830.0f;

constexpr size_t CIE_F_SampleCount	  = 81;
constexpr float CIE_F_WavelengthStart = 380.0f;
constexpr float CIE_F_WavelengthEnd	  = 780.0f;

/////// D65
static float D65_data[CIE_SampleCount] = {
	0.000341f, 0.016643f, 0.032945f, 0.117652f,
	0.20236f, 0.286447f, 0.370535f, 0.385011f,
	0.399488f, 0.424302f, 0.449117f, 0.45775f,
	0.466383f, 0.493637f, 0.520891f, 0.510323f,
	0.499755f, 0.523118f, 0.546482f, 0.687015f,
	0.827549f, 0.871204f, 0.91486f, 0.924589f,
	0.934318f, 0.90057f, 0.866823f, 0.957736f,
	1.04865f, 1.10936f, 1.17008f, 1.1741f,
	1.17812f, 1.16336f, 1.14861f, 1.15392f,
	1.15923f, 1.12367f, 1.08811f, 1.09082f,
	1.09354f, 1.08578f, 1.07802f, 1.06296f,
	1.0479f, 1.06239f, 1.07689f, 1.06047f,
	1.04405f, 1.04225f, 1.04046f, 1.02023f,
	1.0f, 0.981671f, 0.963342f, 0.960611f,
	0.95788f, 0.922368f, 0.886856f, 0.893459f,
	0.900062f, 0.898026f, 0.895991f, 0.886489f,
	0.876987f, 0.854936f, 0.832886f, 0.834939f,
	0.836992f, 0.81863f, 0.800268f, 0.801207f,
	0.802146f, 0.812462f, 0.822778f, 0.80281f,
	0.782842f, 0.740027f, 0.697213f, 0.706652f,
	0.716091f, 0.72979f, 0.74349f, 0.679765f,
	0.61604f, 0.657448f, 0.698856f, 0.724863f,
	0.75087f, 0.693398f, 0.635927f, 0.550054f,
	0.464182f, 0.566118f, 0.668054f, 0.650941f,
	0.633828f, 0.638434f, 0.64304f, 0.618779f,
	0.594519f, 0.557054f, 0.51959f, 0.546998f,
	0.574406f, 0.588765f, 0.603125f
};
/////// A
static float A_data[CIE_SampleCount] = {
	0.00930483f, 0.0112821f, 0.0135769f, 0.0162219f,
	0.0192508f, 0.022698f, 0.0265981f, 0.0309861f,
	0.0358968f, 0.0413648f, 0.0474238f, 0.054107f,
	0.0614462f, 0.069472f, 0.0782135f, 0.087698f,
	0.097951f, 0.108996f, 0.120853f, 0.133543f,
	0.14708f, 0.16148f, 0.176753f, 0.192907f,
	0.20995f, 0.227883f, 0.246709f, 0.266425f,
	0.287027f, 0.308508f, 0.330859f, 0.354068f,
	0.378121f, 0.403002f, 0.428693f, 0.455174f,
	0.482423f, 0.510418f, 0.539132f, 0.568539f,
	0.598611f, 0.62932f, 0.660635f, 0.692525f,
	0.724959f, 0.757903f, 0.791326f, 0.825193f,
	0.85947f, 0.894124f, 0.92912f, 0.964423f,
	1.0f, 1.03582f, 1.07184f, 1.10803f,
	1.14436f, 1.1808f, 1.21731f, 1.25386f,
	1.29043f, 1.32697f, 1.36346f, 1.39988f,
	1.43618f, 1.47235f, 1.50836f, 1.54418f,
	1579.79f, 1.61516f, 1.65028f, 1.6851f,
	1.71963f, 1.75383f, 1.78769f, 1.82118f,
	1.85429f, 1.88701f, 1.91931f, 1.95118f,
	1.98261f, 2.01359f, 2.04409f, 2.07411f,
	2.10365f, 2.13268f, 2.1612f, 2.1892f,
	2.21667f, 2.24361f, 2.27f, 2.29585f,
	2.32115f, 2.34589f, 2.37008f, 2.3937f,
	2.41675f
};

/////// C
static float C_data[CIE_SampleCount] = {
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0001f, 0.002f, 0.004f, 0.0155f,
	0.027f, 0.0485f, 0.07f, 0.0995f,
	0.129f, 0.172f, 0.214f, 0.275f,
	0.33f, 0.3992f, 0.474f, 0.5517f,
	0.633f, 0.7181f, 0.806f, 0.8953f,
	0.981f, 1.058f, 1.124f, 1.1775f,
	1.215f, 1.2345f, 1.24f, 1.236f,
	1.231f, 1.233f, 1.238f, 1.2409f,
	1.239f, 1.2292f, 1.207f, 1.169f,
	1.121f, 1.0698f, 1.023f, 0.9881f,
	0.969f, 0.9678f, 0.98f, 0.9994f,
	1.021f, 1.0395f, 1.052f, 1.0567f,
	1.053f, 1.0411f, 1.023f, 1.0015f,
	0.978f, 0.9543f, 0.932f, 0.9122f,
	0.897f, 0.8883f, 0.884f, 0.8819f,
	0.881f, 0.8806f, 0.88f, 0.8786f,
	0.878f, 0.8799f, 0.882f, 0.882f,
	0.879f, 0.8722f, 0.863f, 0.853f,
	0.84f, 0.8221f, 0.802f, 0.7824f,
	0.763f, 0.7436f, 0.724f, 0.704f,
	0.683f, 0.663f, 0.644f, 0.628f,
	0.615f, 0.602f, 0.592f, 0.585f,
	0.581f, 0.58f, 0.582f, 0.585f,
	0.591f
};

/////// D50
static float D50_data[CIE_SampleCount] = {
	0.00019f, 0.01035f, 0.02051f, 49.14f,
	0.07778f, 0.11263f, 0.14748f, 0.16348f,
	0.17948f, 0.19479f, 0.2101f, 0.22476f,
	0.23942f, 0.25451f, 0.26961f, 0.25724f,
	0.24488f, 0.27179f, 0.29871f, 0.39589f,
	0.49308f, 0.5291f, 0.56513f, 0.58273f,
	0.60034f, 0.58926f, 0.57818f, 0.66321f,
	0.74825f, 0.81036f, 0.87247f, 0.8893f,
	0.90612f, 0.9099f, 0.91368f, 0.93238f,
	0.95109f, 0.93536f, 0.91963f, 0.93843f,
	0.95724f, 0.96169f, 0.96613f, 0.96871f,
	0.97129f, 0.99614f, 1.02099f, 1.01427f,
	1.00755f, 1.01536f, 1.02317f, 1.01159f,
	1.0f, 0.98868f, 0.97735f, 0.98327f,
	0.98918f, 0.96208f, 0.93499f, 0.95593f,
	0.97688f, 0.98478f, 0.99269f, 0.99155f,
	0.99042f, 0.97382f, 0.95722f, 0.9729f,
	0.98857f, 0.97262f, 0.95667f, 0.96929f,
	0.9819f, 1.00597f, 1.03003f, 1.01068f,
	0.99133f, 0.93257f, 0.87381f, 0.89492f,
	0.91604f, 0.92246f, 0.92889f, 0.84872f,
	0.76854f, 0.81683f, 0.86511f, 0.89546f,
	0.9258f, 0.85405f, 0.7823f, 0.67961f,
	0.57692f, 0.70307f, 0.82923f, 0.80599f,
	0.78274f
};

/////// D55
static float D55_data[CIE_SampleCount] = {
	0.00024f, 0.01048f, 0.02072f, 0.06648f,
	0.11224f, 0.15936f, 0.20647f, 0.22266f,
	0.23885f, 0.25851f, 0.27817f, 0.29219f,
	0.30621f, 0.32464f, 0.34308f, 0.33446f,
	0.32584f, 0.35335f, 0.38087f, 0.49518f,
	0.60949f, 0.64751f, 0.68554f, 0.70065f,
	0.71577f, 0.69746f, 0.67914f, 0.7676f,
	0.85605f, 0.91799f, 0.97993f, 0.99228f,
	1.00463f, 1.00188f, 0.99913f, 1.01326f,
	1.02739f, 1.00409f, 0.98078f, 0.99379f,
	1.0068f, 1.00688f, 1.00695f, 1.00341f,
	0.99987f, 1.02098f, 1.0421f, 1.03156f,
	1.02102f, 1.02535f, 1.02968f, 1.01484f,
	1.0f, 0.98608f, 0.97216f, 0.97482f,
	0.97749f, 0.9459f, 0.91432f, 0.92926f,
	0.94419f, 0.9478f, 0.9514f, 0.9468f,
	0.9422f, 0.92334f, 0.90448f, 0.91389f,
	0.9233f, 0.90592f, 0.88854f, 0.89586f,
	0.90317f, 0.92133f, 0.9395f, 0.91953f,
	0.89956f, 0.84817f, 0.79677f, 0.81258f,
	0.8284f, 0.83842f, 0.84844f, 0.77539f,
	0.70235f, 0.74768f, 0.79301f, 0.82147f,
	0.84993f, 0.78437f, 0.7188f, 0.62337f,
	0.52793f, 0.6436f, 0.75927f, 0.73872f,
	0.71818f
};

/////// D75
static float D75_data[CIE_SampleCount] = {
	0.00043f, 0.02588f, 0.05133f, 0.1747f,
	0.29808f, 0.42369f, 0.5493f, 0.56095f,
	0.57259f, 0.6f, 0.6274f, 0.62861f,
	0.62982f, 0.66647f, 0.70312f, 0.68507f,
	0.66703f, 0.68333f, 0.69963f, 0.85946f,
	1.01929f, 1.06911f, 1.11894f, 1.12346f,
	1.12798f, 1.07945f, 1.03092f, 1.12145f,
	1.21198f, 1.27104f, 1.3301f, 1.32682f,
	1.32355f, 1.29838f, 1.27322f, 1.27061f,
	1.268f, 1.22291f, 1.17783f, 1.17186f,
	1.16589f, 1.15146f, 1.13702f, 1.11181f,
	1.08659f, 1.09552f, 1.10445f, 1.08367f,
	1062.89f, 1.05596f, 1.04904f, 1.02452f,
	1.0f, 0.97808f, 0.95616f, 0.94914f,
	0.94213f, 0.90605f, 0.86997f, 0.87112f,
	0.87227f, 0.86684f, 0.8614f, 0.84861f,
	0.83581f, 0.81164f, 0.78747f, 0.78587f,
	0.78428f, 0.76614f, 0.74801f, 0.74562f,
	0.74324f, 0.74873f, 0.75422f, 0.73499f,
	0.71576f, 0.67714f, 0.63852f, 0.64464f,
	0.65076f, 0.66573f, 0.6807f, 0.62256f,
	0.56443f, 0.60343f, 0.64242f, 0.66697f,
	0.69151f, 0.6389f, 0.58629f, 0.50623f,
	0.42617f, 0.51985f, 0.61352f, 0.59838f,
	0.58324f
};

/////// F1-F12
static float F1_data[CIE_F_SampleCount] = {
	0.0187f, 0.0236f, 0.0294f, 0.0347f,
	0.0517f, 0.1949f, 0.0613f, 0.0624f,
	0.0701f, 0.0779f, 0.0856f, 0.4367f,
	0.1694f, 0.1072f, 0.1135f, 0.1189f,
	0.1237f, 0.1275f, 0.13f, 0.1315f,
	0.1323f, 0.1317f, 0.1313f, 0.1285f,
	0.1252f, 0.122f, 0.1183f, 0.115f,
	0.1122f, 0.1105f, 0.1103f, 0.1118f,
	0.1153f, 0.2774f, 0.1705f, 0.1355f,
	0.1433f, 0.1501f, 0.1552f, 0.1829f,
	0.1955f, 0.1548f, 0.1491f, 0.1415f,
	0.1322f, 0.1219f, 0.1112f, 0.1003f,
	0.0895f, 0.0796f, 0.0702f, 0.062f,
	0.0542f, 0.0473f, 0.0415f, 0.0364f,
	0.032f, 0.0281f, 0.0247f, 0.0218f,
	0.0193f, 0.0172f, 0.0167f, 0.0143f,
	0.0129f, 0.0119f, 0.0108f, 0.0096f,
	0.0088f, 0.0081f, 0.0077f, 0.0075f,
	0.0073f, 0.0068f, 0.0069f, 0.0064f,
	0.0068f, 0.0069f, 0.0061f, 0.0052f,
	0.0043f
};

static float F2_data[CIE_F_SampleCount] = {
	0.0118f, 0.0148f, 0.0184f, 0.0215f,
	0.0344f, 0.1569f, 0.0385f, 0.0374f,
	0.0419f, 0.0462f, 0.0506f, 0.3498f,
	0.1181f, 0.0627f, 0.0663f, 0.0693f,
	0.0719f, 0.074f, 0.0754f, 0.0762f,
	0.0765f, 0.0762f, 0.0762f, 0.0745f,
	0.0728f, 0.0715f, 0.0705f, 0.0704f,
	0.0716f, 0.0747f, 0.0804f, 0.0888f,
	0.1001f, 0.2488f, 0.1664f, 0.1459f,
	0.1616f, 0.1756f, 0.1862f, 0.2147f,
	0.2279f, 0.1929f, 0.1866f, 0.1773f,
	0.1654f, 0.1521f, 0.138f, 0.1236f,
	0.1095f, 0.0965f, 0.084f, 0.0732f,
	0.0631f, 0.0543f, 0.0468f, 0.0402f,
	0.0345f, 0.0296f, 0.0255f, 0.0219f,
	0.0189f, 0.0164f, 0.0153f, 0.0127f,
	0.011f, 0.0099f, 0.0088f, 0.0076f,
	0.0068f, 0.0061f, 0.0056f, 0.0054f,
	0.0051f, 0.0047f, 0.0047f, 0.0043f,
	0.0046f, 0.0047f, 0.004f, 0.0033f,
	0.0027f
};

static float F3_data[CIE_F_SampleCount] = {
	0.0082f, 0.0102f, 0.0126f, 0.0144f,
	0.0257f, 0.1436f, 0.027f, 0.0245f,
	0.0273f, 0.03f, 0.0328f, 0.3185f,
	0.0947f, 0.0402f, 0.0425f, 0.0444f,
	0.0459f, 0.0472f, 0.048f, 0.0486f,
	0.0487f, 0.0485f, 0.0488f, 0.0477f,
	0.0467f, 0.0462f, 0.0462f, 0.0473f,
	0.0499f, 0.0548f, 0.0625f, 0.0734f,
	0.0878f, 0.2382f, 0.1614f, 0.1459f,
	0.1663f, 0.1849f, 0.1995f, 0.2311f,
	0.2469f, 0.2141f, 0.2085f, 0.1993f,
	0.1867f, 0.1722f, 0.1565f, 0.1404f,
	0.1245f, 0.1095f, 0.0951f, 0.0827f,
	0.0711f, 0.0609f, 0.0522f, 0.0445f,
	0.038f, 0.0323f, 0.0275f, 0.0233f,
	0.0199f, 0.017f, 0.0155f, 0.0127f,
	0.0109f, 0.0096f, 0.0083f, 0.0071f,
	0.0062f, 0.0054f, 0.0049f, 0.0046f,
	0.0043f, 0.0039f, 0.0039f, 0.0035f,
	0.0038f, 0.0039f, 0.0033f, 0.0028f,
	0.0021f
};

static float F4_data[CIE_F_SampleCount] = {
	0.0057f, 0.007f, 0.0087f, 0.0098f,
	0.0201f, 0.1375f, 0.0195f, 0.0159f,
	0.0176f, 0.0193f, 0.021f, 0.3028f,
	0.0803f, 0.0255f, 0.027f, 0.0282f,
	0.0291f, 0.0299f, 0.0304f, 0.0308f,
	0.0309f, 0.0309f, 0.0314f, 0.0306f,
	0.03f, 0.0298f, 0.0301f, 0.0314f,
	0.0341f, 0.039f, 0.0469f, 0.0581f,
	0.0732f, 0.2259f, 0.1511f, 0.1388f,
	0.1633f, 0.1868f, 0.2064f, 0.2428f,
	0.2626f, 0.2328f, 0.2294f, 0.2214f,
	0.2091f, 0.1943f, 0.1774f, 0.16f,
	0.1442f, 0.1256f, 0.1093f, 0.0952f,
	0.0818f, 0.0701f, 0.06f, 0.0511f,
	0.0436f, 0.0369f, 0.0313f, 0.0264f,
	0.0224f, 0.0191f, 0.017f, 0.0139f,
	0.0118f, 0.0103f, 0.0088f, 0.0074f,
	0.0064f, 0.0054f, 0.0049f, 0.0046f,
	0.0042f, 0.0037f, 0.0037f, 0.0033f,
	0.0035f, 0.0036f, 0.0031f, 0.0026f,
	0.0019f
};

static float F5_data[CIE_F_SampleCount] = {
	0.0187f, 0.0235f, 0.0292f, 0.0345f,
	0.051f, 0.1891f, 0.06f, 0.0611f,
	0.0685f, 0.0758f, 0.0831f, 0.4076f,
	0.1606f, 0.1032f, 0.1091f, 0.114f,
	0.1183f, 0.1217f, 0.124f, 0.1254f,
	0.1258f, 0.1252f, 0.1247f, 0.122f,
	0.1189f, 0.1161f, 0.1133f, 0.111f,
	0.1096f, 0.1097f, 0.1116f, 0.1154f,
	0.1212f, 0.2778f, 0.1773f, 0.1447f,
	0.152f, 0.1577f, 0.161f, 0.1854f,
	0.195f, 0.1539f, 0.1464f, 0.1372f,
	0.1269f, 0.1157f, 0.1045f, 0.0935f,
	0.0829f, 0.0732f, 0.0641f, 0.0563f,
	0.049f, 0.0426f, 0.0372f, 0.0325f,
	0.0283f, 0.0249f, 0.0219f, 0.0193f,
	0.0171f, 0.0152f, 0.0143f, 0.0126f,
	0.0113f, 0.0105f, 0.0096f, 0.0085f,
	0.0078f, 0.0072f, 0.0068f, 0.0067f,
	0.0065f, 0.0061f, 0.0062f, 0.0059f,
	0.0062f, 0.0064f, 0.0055f, 0.0047f,
	0.004f
};

static float F6_data[CIE_F_SampleCount] = {
	0.0105f, 0.0131f, 0.0163f, 0.019f,
	0.0311f, 0.148f, 0.0343f, 0.033f,
	0.0368f, 0.0407f, 0.0445f, 0.3261f,
	0.1074f, 0.0548f, 0.0578f, 0.0603f,
	0.0625f, 0.0641f, 0.0652f, 0.0658f,
	0.0659f, 0.0656f, 0.0656f, 0.0642f,
	0.0628f, 0.062f, 0.0619f, 0.063f,
	0.066f, 0.0712f, 0.0794f, 0.0907f,
	0.1049f, 0.2522f, 0.1746f, 0.1563f,
	0.1722f, 0.1853f, 0.1943f, 0.2197f,
	0.2301f, 0.1941f, 0.1856f, 0.1742f,
	0.1609f, 0.1464f, 0.1315f, 0.1168f,
	0.1025f, 0.0896f, 0.0774f, 0.0669f,
	0.0571f, 0.0487f, 0.0416f, 0.0355f,
	0.0302f, 0.0257f, 0.022f, 0.0187f,
	0.016f, 0.0137f, 0.0129f, 0.0105f,
	0.0091f, 0.0081f, 0.0071f, 0.0061f,
	0.0054f, 0.0048f, 0.0044f, 0.0043f,
	0.004f, 0.0037f, 0.0038f, 0.0035f,
	0.0039f, 0.0041f, 0.0033f, 0.0026f,
	0.0021f
};

static float F7_data[CIE_F_SampleCount] = {
	0.0256f, 0.0318f, 0.0384f, 0.0453f,
	0.0615f, 0.1937f, 0.0737f, 0.0705f,
	0.0771f, 0.0841f, 0.0915f, 0.4414f,
	0.1752f, 0.1135f, 0.12f, 0.1258f,
	0.1308f, 0.1345f, 0.1371f, 0.1388f,
	0.1395f, 0.1393f, 0.1382f, 0.1364f,
	0.1343f, 0.1325f, 0.1308f, 0.1293f,
	0.1278f, 0.126f, 0.1244f, 0.1233f,
	0.1226f, 0.2952f, 0.1705f, 0.1244f,
	0.1258f, 0.1272f, 0.1283f, 0.1546f,
	0.1675f, 0.1283f, 0.1267f, 0.1245f,
	0.1219f, 0.1189f, 0.116f, 0.1135f,
	0.1112f, 0.1095f, 0.1076f, 0.1042f,
	0.1011f, 0.1004f, 0.1002f, 0.1011f,
	0.0987f, 0.0865f, 0.0727f, 0.0644f,
	0.0583f, 0.0541f, 0.0504f, 0.0457f,
	0.0412f, 0.0377f, 0.0346f, 0.0308f,
	0.0273f, 0.0247f, 0.0225f, 0.0206f,
	0.019f, 0.0175f, 0.0162f, 0.0154f,
	0.0145f, 0.0132f, 0.0117f, 0.0099f,
	0.0081f
};

static float F8_data[CIE_F_SampleCount] = {
	0.0121f, 0.015f, 0.0181f, 0.0213f,
	0.0317f, 0.1308f, 0.0383f, 0.0345f,
	0.0386f, 0.0442f, 0.0509f, 0.341f,
	0.1242f, 0.0768f, 0.086f, 0.0946f,
	0.1024f, 0.1084f, 0.1133f, 0.1171f,
	0.1198f, 0.1217f, 0.1228f, 0.1232f,
	0.1235f, 0.1244f, 0.1255f, 0.1268f,
	0.1277f, 0.1272f, 0.126f, 0.1243f,
	0.1222f, 0.2896f, 0.1651f, 0.1179f,
	0.1176f, 0.1177f, 0.1184f, 0.1461f,
	0.1611f, 0.1234f, 0.1253f, 0.1272f,
	0.1292f, 0.1312f, 0.1334f, 0.1361f,
	0.1387f, 0.1407f, 0.142f, 0.1416f,
	0.1413f, 0.1434f, 0.145f, 0.1446f,
	0.14f, 0.1258f, 0.1099f, 0.0998f,
	0.0922f, 0.0862f, 0.0807f, 0.0739f,
	0.0671f, 0.0616f, 0.0563f, 0.0503f,
	0.0446f, 0.0402f, 0.0366f, 0.0336f,
	0.0309f, 0.0285f, 0.0265f, 0.0251f,
	0.0237f, 0.0215f, 0.0189f, 0.0161f,
	0.0132f
};

static float F9_data[CIE_F_SampleCount] = {
	0.009f, 0.0112f, 0.0136f, 0.016f,
	0.0259f, 0.128f, 0.0305f, 0.0256f,
	0.0286f, 0.033f, 0.0382f, 0.3262f,
	0.1077f, 0.0584f, 0.0657f, 0.0725f,
	0.0786f, 0.0835f, 0.0875f, 0.0906f,
	0.0931f, 0.0948f, 0.0961f, 0.0968f,
	0.0974f, 0.0988f, 0.1004f, 0.1026f,
	0.1048f, 0.1063f, 0.1076f, 0.1096f,
	0.1118f, 0.2771f, 0.1629f, 0.1228f,
	0.1274f, 0.1321f, 0.1365f, 0.1657f,
	0.1814f, 0.1455f, 0.1465f, 0.1466f,
	0.1461f, 0.145f, 0.1439f, 0.144f,
	0.1447f, 0.1462f, 0.1472f, 0.1455f,
	0.144f, 0.1458f, 0.1488f, 0.1551f,
	0.1547f, 0.132f, 0.1057f, 0.0918f,
	0.0825f, 0.0757f, 0.0703f, 0.0635f,
	0.0572f, 0.0525f, 0.048f, 0.0429f,
	0.038f, 0.0343f, 0.0312f, 0.0286f,
	0.0264f, 0.0243f, 0.0226f, 0.0214f,
	0.0202f, 0.0183f, 0.0161f, 0.0138f,
	0.0112f
};

static float F10_data[CIE_F_SampleCount] = {
	0.0111f, 0.0063f, 0.0062f, 0.0057f,
	0.0148f, 0.1216f, 0.0212f, 0.027f,
	0.0374f, 0.0514f, 0.0675f, 0.3439f,
	0.1486f, 0.104f, 0.1076f, 0.1067f,
	0.1011f, 0.0927f, 0.0829f, 0.0729f,
	0.0791f, 0.1664f, 0.1673f, 0.1044f,
	0.0594f, 0.0334f, 0.0235f, 0.0188f,
	0.0159f, 0.0147f, 0.018f, 0.0571f,
	0.4098f, 0.7369f, 0.3361f, 0.0824f,
	0.0338f, 0.0247f, 0.0214f, 0.0486f,
	0.1145f, 0.1479f, 0.1216f, 0.0897f,
	0.0652f, 0.0881f, 0.4412f, 0.3455f,
	0.1209f, 0.1215f, 0.1052f, 0.0443f,
	0.0195f, 0.0219f, 0.0319f, 0.0277f,
	0.0229f, 0.02f, 0.0152f, 0.0135f,
	0.0147f, 0.0179f, 0.0174f, 0.0102f,
	0.0114f, 0.0332f, 0.0449f, 0.0205f,
	0.0049f, 0.0024f, 0.0021f, 0.0021f,
	0.0024f, 0.0024f, 0.0021f, 0.0017f,
	0.0021f, 0.0022f, 0.0017f, 0.0012f,
	0.0009f
};

static float F11_data[CIE_F_SampleCount] = {
	0.0091f, 0.0063f, 0.0046f, 0.0037f,
	0.0129f, 0.1268f, 0.0159f, 0.0179f,
	0.0246f, 0.0333f, 0.0449f, 0.3394f,
	0.1213f, 0.0695f, 0.0719f, 0.0712f,
	0.0672f, 0.0613f, 0.0546f, 0.0479f,
	0.0566f, 0.1429f, 0.1496f, 0.0897f,
	0.0472f, 0.0233f, 0.0147f, 0.011f,
	0.0089f, 0.0083f, 0.0118f, 0.049f,
	0.3959f, 0.7284f, 0.3261f, 0.0752f,
	0.0283f, 0.0196f, 0.0167f, 0.0443f,
	0.1128f, 0.1476f, 0.1273f, 0.0974f,
	0.0733f, 0.0972f, 0.5527f, 0.4258f,
	0.1318f, 0.1316f, 0.1226f, 0.0511f,
	0.0207f, 0.0234f, 0.0358f, 0.0301f,
	0.0248f, 0.0214f, 0.0154f, 0.0133f,
	0.0146f, 0.0194f, 0.02f, 0.012f,
	0.0135f, 0.041f, 0.0558f, 0.0251f,
	0.0057f, 0.0027f, 0.0023f, 0.0021f,
	0.0024f, 0.0024f, 0.002f, 0.0024f,
	0.0032f, 0.0026f, 0.0016f, 0.0012f,
	0.0009f
};

static float F12_data[CIE_F_SampleCount] = {
	0.0096f, 0.0064f, 0.0045f, 0.0033f,
	0.0119f, 0.1248f, 0.0112f, 0.0094f,
	0.0108f, 0.0137f, 0.0178f, 0.2905f,
	0.079f, 0.0265f, 0.0271f, 0.0265f,
	0.0249f, 0.0233f, 0.021f, 0.0191f,
	0.0301f, 0.1083f, 0.1188f, 0.0688f,
	0.0343f, 0.0149f, 0.0092f, 0.0071f,
	0.006f, 0.0063f, 0.011f, 0.0456f,
	0.344f, 0.654f, 0.2948f, 0.0716f,
	0.0308f, 0.0247f, 0.0227f, 0.0509f,
	0.1196f, 0.1532f, 0.1427f, 0.1186f,
	0.0928f, 0.1231f, 0.6853f, 0.5302f,
	0.1467f, 0.1438f, 0.1471f, 0.0646f,
	0.0257f, 0.0275f, 0.0418f, 0.0344f,
	0.0281f, 0.0242f, 0.0164f, 0.0136f,
	0.0149f, 0.0214f, 0.0234f, 0.0142f,
	0.0161f, 0.0504f, 0.0698f, 0.0319f,
	0.0071f, 0.003f, 0.0026f, 0.0023f,
	0.0028f, 0.0028f, 0.0021f, 0.0017f,
	0.0021f, 0.0019f, 0.0015f, 0.001f,
	0.0005f
};