/////////////////////////////////////////////////////////////
// Vector Math Tests
/////////////////////////////////////////////////////////////

// Swizzle
void test_float4_swizzle()
{
	test.beginTest("float4_swizzle");
	float4 a = float4(1, 2, 3, 4);
	// float4 -> float4 swizzles.
	test.expectEq(a.xxxx, float4(1,1,1,1));
	test.expectEq(a.xxxy, float4(1,1,1,2));
	test.expectEq(a.xxxz, float4(1,1,1,3));
	test.expectEq(a.xxxw, float4(1,1,1,4));
	test.expectEq(a.xxyx, float4(1,1,2,1));
	test.expectEq(a.xxyy, float4(1,1,2,2));
	test.expectEq(a.xxyz, float4(1,1,2,3));
	test.expectEq(a.xxyw, float4(1,1,2,4));
	test.expectEq(a.xxzx, float4(1,1,3,1));
	test.expectEq(a.xxzy, float4(1,1,3,2));
	test.expectEq(a.xxzz, float4(1,1,3,3));
	test.expectEq(a.xxzw, float4(1,1,3,4));
	test.expectEq(a.xxwx, float4(1,1,4,1));
	test.expectEq(a.xxwy, float4(1,1,4,2));
	test.expectEq(a.xxwz, float4(1,1,4,3));
	test.expectEq(a.xxww, float4(1,1,4,4));
	test.expectEq(a.xyxx, float4(1,2,1,1));
	test.expectEq(a.xyxy, float4(1,2,1,2));
	test.expectEq(a.xyxz, float4(1,2,1,3));
	test.expectEq(a.xyxw, float4(1,2,1,4));
	test.expectEq(a.xyyx, float4(1,2,2,1));
	test.expectEq(a.xyyy, float4(1,2,2,2));
	test.expectEq(a.xyyz, float4(1,2,2,3));
	test.expectEq(a.xyyw, float4(1,2,2,4));
	test.expectEq(a.xyzx, float4(1,2,3,1));
	test.expectEq(a.xyzy, float4(1,2,3,2));
	test.expectEq(a.xyzz, float4(1,2,3,3));
	test.expectEq(a.xyzw, float4(1,2,3,4));
	test.expectEq(a.xywx, float4(1,2,4,1));
	test.expectEq(a.xywy, float4(1,2,4,2));
	test.expectEq(a.xywz, float4(1,2,4,3));
	test.expectEq(a.xyww, float4(1,2,4,4));
	test.expectEq(a.xzxx, float4(1,3,1,1));
	test.expectEq(a.xzxy, float4(1,3,1,2));
	test.expectEq(a.xzxz, float4(1,3,1,3));
	test.expectEq(a.xzxw, float4(1,3,1,4));
	test.expectEq(a.xzyx, float4(1,3,2,1));
	test.expectEq(a.xzyy, float4(1,3,2,2));
	test.expectEq(a.xzyz, float4(1,3,2,3));
	test.expectEq(a.xzyw, float4(1,3,2,4));
	test.expectEq(a.xzzx, float4(1,3,3,1));
	test.expectEq(a.xzzy, float4(1,3,3,2));
	test.expectEq(a.xzzz, float4(1,3,3,3));
	test.expectEq(a.xzzw, float4(1,3,3,4));
	test.expectEq(a.xzwx, float4(1,3,4,1));
	test.expectEq(a.xzwy, float4(1,3,4,2));
	test.expectEq(a.xzwz, float4(1,3,4,3));
	test.expectEq(a.xzww, float4(1,3,4,4));
	test.expectEq(a.xwxx, float4(1,4,1,1));
	test.expectEq(a.xwxy, float4(1,4,1,2));
	test.expectEq(a.xwxz, float4(1,4,1,3));
	test.expectEq(a.xwxw, float4(1,4,1,4));
	test.expectEq(a.xwyx, float4(1,4,2,1));
	test.expectEq(a.xwyy, float4(1,4,2,2));
	test.expectEq(a.xwyz, float4(1,4,2,3));
	test.expectEq(a.xwyw, float4(1,4,2,4));
	test.expectEq(a.xwzx, float4(1,4,3,1));
	test.expectEq(a.xwzy, float4(1,4,3,2));
	test.expectEq(a.xwzz, float4(1,4,3,3));
	test.expectEq(a.xwzw, float4(1,4,3,4));
	test.expectEq(a.xwwx, float4(1,4,4,1));
	test.expectEq(a.xwwy, float4(1,4,4,2));
	test.expectEq(a.xwwz, float4(1,4,4,3));
	test.expectEq(a.xwww, float4(1,4,4,4));
	test.expectEq(a.yxxx, float4(2,1,1,1));
	test.expectEq(a.yxxy, float4(2,1,1,2));
	test.expectEq(a.yxxz, float4(2,1,1,3));
	test.expectEq(a.yxxw, float4(2,1,1,4));
	test.expectEq(a.yxyx, float4(2,1,2,1));
	test.expectEq(a.yxyy, float4(2,1,2,2));
	test.expectEq(a.yxyz, float4(2,1,2,3));
	test.expectEq(a.yxyw, float4(2,1,2,4));
	test.expectEq(a.yxzx, float4(2,1,3,1));
	test.expectEq(a.yxzy, float4(2,1,3,2));
	test.expectEq(a.yxzz, float4(2,1,3,3));
	test.expectEq(a.yxzw, float4(2,1,3,4));
	test.expectEq(a.yxwx, float4(2,1,4,1));
	test.expectEq(a.yxwy, float4(2,1,4,2));
	test.expectEq(a.yxwz, float4(2,1,4,3));
	test.expectEq(a.yxww, float4(2,1,4,4));
	test.expectEq(a.yyxx, float4(2,2,1,1));
	test.expectEq(a.yyxy, float4(2,2,1,2));
	test.expectEq(a.yyxz, float4(2,2,1,3));
	test.expectEq(a.yyxw, float4(2,2,1,4));
	test.expectEq(a.yyyx, float4(2,2,2,1));
	test.expectEq(a.yyyy, float4(2,2,2,2));
	test.expectEq(a.yyyz, float4(2,2,2,3));
	test.expectEq(a.yyyw, float4(2,2,2,4));
	test.expectEq(a.yyzx, float4(2,2,3,1));
	test.expectEq(a.yyzy, float4(2,2,3,2));
	test.expectEq(a.yyzz, float4(2,2,3,3));
	test.expectEq(a.yyzw, float4(2,2,3,4));
	test.expectEq(a.yywx, float4(2,2,4,1));
	test.expectEq(a.yywy, float4(2,2,4,2));
	test.expectEq(a.yywz, float4(2,2,4,3));
	test.expectEq(a.yyww, float4(2,2,4,4));
	test.expectEq(a.yzxx, float4(2,3,1,1));
	test.expectEq(a.yzxy, float4(2,3,1,2));
	test.expectEq(a.yzxz, float4(2,3,1,3));
	test.expectEq(a.yzxw, float4(2,3,1,4));
	test.expectEq(a.yzyx, float4(2,3,2,1));
	test.expectEq(a.yzyy, float4(2,3,2,2));
	test.expectEq(a.yzyz, float4(2,3,2,3));
	test.expectEq(a.yzyw, float4(2,3,2,4));
	test.expectEq(a.yzzx, float4(2,3,3,1));
	test.expectEq(a.yzzy, float4(2,3,3,2));
	test.expectEq(a.yzzz, float4(2,3,3,3));
	test.expectEq(a.yzzw, float4(2,3,3,4));
	test.expectEq(a.yzwx, float4(2,3,4,1));
	test.expectEq(a.yzwy, float4(2,3,4,2));
	test.expectEq(a.yzwz, float4(2,3,4,3));
	test.expectEq(a.yzww, float4(2,3,4,4));
	test.expectEq(a.ywxx, float4(2,4,1,1));
	test.expectEq(a.ywxy, float4(2,4,1,2));
	test.expectEq(a.ywxz, float4(2,4,1,3));
	test.expectEq(a.ywxw, float4(2,4,1,4));
	test.expectEq(a.ywyx, float4(2,4,2,1));
	test.expectEq(a.ywyy, float4(2,4,2,2));
	test.expectEq(a.ywyz, float4(2,4,2,3));
	test.expectEq(a.ywyw, float4(2,4,2,4));
	test.expectEq(a.ywzx, float4(2,4,3,1));
	test.expectEq(a.ywzy, float4(2,4,3,2));
	test.expectEq(a.ywzz, float4(2,4,3,3));
	test.expectEq(a.ywzw, float4(2,4,3,4));
	test.expectEq(a.ywwx, float4(2,4,4,1));
	test.expectEq(a.ywwy, float4(2,4,4,2));
	test.expectEq(a.ywwz, float4(2,4,4,3));
	test.expectEq(a.ywww, float4(2,4,4,4));
	test.expectEq(a.zxxx, float4(3,1,1,1));
	test.expectEq(a.zxxy, float4(3,1,1,2));
	test.expectEq(a.zxxz, float4(3,1,1,3));
	test.expectEq(a.zxxw, float4(3,1,1,4));
	test.expectEq(a.zxyx, float4(3,1,2,1));
	test.expectEq(a.zxyy, float4(3,1,2,2));
	test.expectEq(a.zxyz, float4(3,1,2,3));
	test.expectEq(a.zxyw, float4(3,1,2,4));
	test.expectEq(a.zxzx, float4(3,1,3,1));
	test.expectEq(a.zxzy, float4(3,1,3,2));
	test.expectEq(a.zxzz, float4(3,1,3,3));
	test.expectEq(a.zxzw, float4(3,1,3,4));
	test.expectEq(a.zxwx, float4(3,1,4,1));
	test.expectEq(a.zxwy, float4(3,1,4,2));
	test.expectEq(a.zxwz, float4(3,1,4,3));
	test.expectEq(a.zxww, float4(3,1,4,4));
	test.expectEq(a.zyxx, float4(3,2,1,1));
	test.expectEq(a.zyxy, float4(3,2,1,2));
	test.expectEq(a.zyxz, float4(3,2,1,3));
	test.expectEq(a.zyxw, float4(3,2,1,4));
	test.expectEq(a.zyyx, float4(3,2,2,1));
	test.expectEq(a.zyyy, float4(3,2,2,2));
	test.expectEq(a.zyyz, float4(3,2,2,3));
	test.expectEq(a.zyyw, float4(3,2,2,4));
	test.expectEq(a.zyzx, float4(3,2,3,1));
	test.expectEq(a.zyzy, float4(3,2,3,2));
	test.expectEq(a.zyzz, float4(3,2,3,3));
	test.expectEq(a.zyzw, float4(3,2,3,4));
	test.expectEq(a.zywx, float4(3,2,4,1));
	test.expectEq(a.zywy, float4(3,2,4,2));
	test.expectEq(a.zywz, float4(3,2,4,3));
	test.expectEq(a.zyww, float4(3,2,4,4));
	test.expectEq(a.zzxx, float4(3,3,1,1));
	test.expectEq(a.zzxy, float4(3,3,1,2));
	test.expectEq(a.zzxz, float4(3,3,1,3));
	test.expectEq(a.zzxw, float4(3,3,1,4));
	test.expectEq(a.zzyx, float4(3,3,2,1));
	test.expectEq(a.zzyy, float4(3,3,2,2));
	test.expectEq(a.zzyz, float4(3,3,2,3));
	test.expectEq(a.zzyw, float4(3,3,2,4));
	test.expectEq(a.zzzx, float4(3,3,3,1));
	test.expectEq(a.zzzy, float4(3,3,3,2));
	test.expectEq(a.zzzz, float4(3,3,3,3));
	test.expectEq(a.zzzw, float4(3,3,3,4));
	test.expectEq(a.zzwx, float4(3,3,4,1));
	test.expectEq(a.zzwy, float4(3,3,4,2));
	test.expectEq(a.zzwz, float4(3,3,4,3));
	test.expectEq(a.zzww, float4(3,3,4,4));
	test.expectEq(a.zwxx, float4(3,4,1,1));
	test.expectEq(a.zwxy, float4(3,4,1,2));
	test.expectEq(a.zwxz, float4(3,4,1,3));
	test.expectEq(a.zwxw, float4(3,4,1,4));
	test.expectEq(a.zwyx, float4(3,4,2,1));
	test.expectEq(a.zwyy, float4(3,4,2,2));
	test.expectEq(a.zwyz, float4(3,4,2,3));
	test.expectEq(a.zwyw, float4(3,4,2,4));
	test.expectEq(a.zwzx, float4(3,4,3,1));
	test.expectEq(a.zwzy, float4(3,4,3,2));
	test.expectEq(a.zwzz, float4(3,4,3,3));
	test.expectEq(a.zwzw, float4(3,4,3,4));
	test.expectEq(a.zwwx, float4(3,4,4,1));
	test.expectEq(a.zwwy, float4(3,4,4,2));
	test.expectEq(a.zwwz, float4(3,4,4,3));
	test.expectEq(a.zwww, float4(3,4,4,4));
	test.expectEq(a.wxxx, float4(4,1,1,1));
	test.expectEq(a.wxxy, float4(4,1,1,2));
	test.expectEq(a.wxxz, float4(4,1,1,3));
	test.expectEq(a.wxxw, float4(4,1,1,4));
	test.expectEq(a.wxyx, float4(4,1,2,1));
	test.expectEq(a.wxyy, float4(4,1,2,2));
	test.expectEq(a.wxyz, float4(4,1,2,3));
	test.expectEq(a.wxyw, float4(4,1,2,4));
	test.expectEq(a.wxzx, float4(4,1,3,1));
	test.expectEq(a.wxzy, float4(4,1,3,2));
	test.expectEq(a.wxzz, float4(4,1,3,3));
	test.expectEq(a.wxzw, float4(4,1,3,4));
	test.expectEq(a.wxwx, float4(4,1,4,1));
	test.expectEq(a.wxwy, float4(4,1,4,2));
	test.expectEq(a.wxwz, float4(4,1,4,3));
	test.expectEq(a.wxww, float4(4,1,4,4));
	test.expectEq(a.wyxx, float4(4,2,1,1));
	test.expectEq(a.wyxy, float4(4,2,1,2));
	test.expectEq(a.wyxz, float4(4,2,1,3));
	test.expectEq(a.wyxw, float4(4,2,1,4));
	test.expectEq(a.wyyx, float4(4,2,2,1));
	test.expectEq(a.wyyy, float4(4,2,2,2));
	test.expectEq(a.wyyz, float4(4,2,2,3));
	test.expectEq(a.wyyw, float4(4,2,2,4));
	test.expectEq(a.wyzx, float4(4,2,3,1));
	test.expectEq(a.wyzy, float4(4,2,3,2));
	test.expectEq(a.wyzz, float4(4,2,3,3));
	test.expectEq(a.wyzw, float4(4,2,3,4));
	test.expectEq(a.wywx, float4(4,2,4,1));
	test.expectEq(a.wywy, float4(4,2,4,2));
	test.expectEq(a.wywz, float4(4,2,4,3));
	test.expectEq(a.wyww, float4(4,2,4,4));
	test.expectEq(a.wzxx, float4(4,3,1,1));
	test.expectEq(a.wzxy, float4(4,3,1,2));
	test.expectEq(a.wzxz, float4(4,3,1,3));
	test.expectEq(a.wzxw, float4(4,3,1,4));
	test.expectEq(a.wzyx, float4(4,3,2,1));
	test.expectEq(a.wzyy, float4(4,3,2,2));
	test.expectEq(a.wzyz, float4(4,3,2,3));
	test.expectEq(a.wzyw, float4(4,3,2,4));
	test.expectEq(a.wzzx, float4(4,3,3,1));
	test.expectEq(a.wzzy, float4(4,3,3,2));
	test.expectEq(a.wzzz, float4(4,3,3,3));
	test.expectEq(a.wzzw, float4(4,3,3,4));
	test.expectEq(a.wzwx, float4(4,3,4,1));
	test.expectEq(a.wzwy, float4(4,3,4,2));
	test.expectEq(a.wzwz, float4(4,3,4,3));
	test.expectEq(a.wzww, float4(4,3,4,4));
	test.expectEq(a.wwxx, float4(4,4,1,1));
	test.expectEq(a.wwxy, float4(4,4,1,2));
	test.expectEq(a.wwxz, float4(4,4,1,3));
	test.expectEq(a.wwxw, float4(4,4,1,4));
	test.expectEq(a.wwyx, float4(4,4,2,1));
	test.expectEq(a.wwyy, float4(4,4,2,2));
	test.expectEq(a.wwyz, float4(4,4,2,3));
	test.expectEq(a.wwyw, float4(4,4,2,4));
	test.expectEq(a.wwzx, float4(4,4,3,1));
	test.expectEq(a.wwzy, float4(4,4,3,2));
	test.expectEq(a.wwzz, float4(4,4,3,3));
	test.expectEq(a.wwzw, float4(4,4,3,4));
	test.expectEq(a.wwwx, float4(4,4,4,1));
	test.expectEq(a.wwwy, float4(4,4,4,2));
	test.expectEq(a.wwwz, float4(4,4,4,3));
	test.expectEq(a.wwww, float4(4,4,4,4));
	// float4 -> float3 swizzles.
	test.expectEq(a.xxx, float3(1,1,1));
	test.expectEq(a.xxy, float3(1,1,2));
	test.expectEq(a.xxz, float3(1,1,3));
	test.expectEq(a.xxw, float3(1,1,4));
	test.expectEq(a.xyx, float3(1,2,1));
	test.expectEq(a.xyy, float3(1,2,2));
	test.expectEq(a.xyz, float3(1,2,3));
	test.expectEq(a.xyw, float3(1,2,4));
	test.expectEq(a.xzx, float3(1,3,1));
	test.expectEq(a.xzy, float3(1,3,2));
	test.expectEq(a.xzz, float3(1,3,3));
	test.expectEq(a.xzw, float3(1,3,4));
	test.expectEq(a.xwx, float3(1,4,1));
	test.expectEq(a.xwy, float3(1,4,2));
	test.expectEq(a.xwz, float3(1,4,3));
	test.expectEq(a.xww, float3(1,4,4));
	test.expectEq(a.yxx, float3(2,1,1));
	test.expectEq(a.yxy, float3(2,1,2));
	test.expectEq(a.yxz, float3(2,1,3));
	test.expectEq(a.yxw, float3(2,1,4));
	test.expectEq(a.yyx, float3(2,2,1));
	test.expectEq(a.yyy, float3(2,2,2));
	test.expectEq(a.yyz, float3(2,2,3));
	test.expectEq(a.yyw, float3(2,2,4));
	test.expectEq(a.yzx, float3(2,3,1));
	test.expectEq(a.yzy, float3(2,3,2));
	test.expectEq(a.yzz, float3(2,3,3));
	test.expectEq(a.yzw, float3(2,3,4));
	test.expectEq(a.ywx, float3(2,4,1));
	test.expectEq(a.ywy, float3(2,4,2));
	test.expectEq(a.ywz, float3(2,4,3));
	test.expectEq(a.yww, float3(2,4,4));
	test.expectEq(a.zxx, float3(3,1,1));
	test.expectEq(a.zxy, float3(3,1,2));
	test.expectEq(a.zxz, float3(3,1,3));
	test.expectEq(a.zxw, float3(3,1,4));
	test.expectEq(a.zyx, float3(3,2,1));
	test.expectEq(a.zyy, float3(3,2,2));
	test.expectEq(a.zyz, float3(3,2,3));
	test.expectEq(a.zyw, float3(3,2,4));
	test.expectEq(a.zzx, float3(3,3,1));
	test.expectEq(a.zzy, float3(3,3,2));
    test.expectEq(a.zzz, float3(3,3,3));
    test.expectEq(a.zzw, float3(3,3,4));
    test.expectEq(a.zwx, float3(3,4,1));
    test.expectEq(a.zwy, float3(3,4,2));
    test.expectEq(a.zwz, float3(3,4,3));
    test.expectEq(a.zww, float3(3,4,4));
    test.expectEq(a.wxx, float3(4,1,1));
    test.expectEq(a.wxy, float3(4,1,2));
    test.expectEq(a.wxz, float3(4,1,3));
    test.expectEq(a.wxw, float3(4,1,4));
    test.expectEq(a.wyx, float3(4,2,1));
    test.expectEq(a.wyy, float3(4,2,2));
    test.expectEq(a.wyz, float3(4,2,3));
    test.expectEq(a.wyw, float3(4,2,4));
    test.expectEq(a.wzx, float3(4,3,1));
    test.expectEq(a.wzy, float3(4,3,2));
    test.expectEq(a.wzz, float3(4,3,3));
    test.expectEq(a.wzw, float3(4,3,4));
    test.expectEq(a.wwx, float3(4,4,1));
    test.expectEq(a.wwy, float3(4,4,2));
    test.expectEq(a.wwz, float3(4,4,3));
    test.expectEq(a.www, float3(4,4,4));
	// float4 -> float2 swizzles.
	test.expectEq(a.xx, float2(1,1));
	test.expectEq(a.xy, float2(1,2));
	test.expectEq(a.xz, float2(1,3));
	test.expectEq(a.xw, float2(1,4));
	test.expectEq(a.yx, float2(2,1));
	test.expectEq(a.yy, float2(2,2));
	test.expectEq(a.yz, float2(2,3));
	test.expectEq(a.yw, float2(2,4));
	test.expectEq(a.zx, float2(3,1));
	test.expectEq(a.zy, float2(3,2));
	test.expectEq(a.zz, float2(3,3));
	test.expectEq(a.zw, float2(3,4));
	test.expectEq(a.wx, float2(4,1));
	test.expectEq(a.wy, float2(4,2));
	test.expectEq(a.wz, float2(4,3));
	test.expectEq(a.ww, float2(4,4));
	// float4 -> scalar swizzles.
	test.expectEq(a.x, 1.0f);
	test.expectEq(a.y, 2.0f);
	test.expectEq(a.z, 3.0f);
	test.expectEq(a.w, 4.0f);
	// float4 array indexing.
	test.expectEq(a[0], 1.0f);
	test.expectEq(a[1], 2.0f);
	test.expectEq(a[2], 3.0f);
	test.expectEq(a[3], 4.0f);
}

// float4 x float4 math
void test_float4_float4_math()
{
	test.beginTest("float4_float4_math");
	float4 a = float4(1, 2, 3, 4);
	float4 b = float4(3, 4, 5, 6);
	float4 c = float4(0, 1, 2, 3);
	test.expectEq(a + b, float4(4, 6, 8, 10));
	test.expectEq(a - b, float4(-2, -2, -2, -2));
	test.expectEq(a * b, float4(3, 8, 15, 24));
	test.expectEq(a / b, float4(1.0/3.0, 2.0/4.0, 3.0/5.0, 4.0/6.0));

	test.expectEq(math.distance(a, b), 4.0f);
	test.expectEq(math.dot(a, b), 50.0f);
	test.expectEq(math.length(a), 5.47722578f);
	test.expectEq(math.normalize(a), float4(0.182574183, 0.365148365, 0.547722578, 0.730296731));
	
	// Test normalize with zero-vector (invalid).
	test.expectEq(math.normalize(float4(0)), float4(0));
	
	test.expectEq(math.clamp(a, float4(0.5,2.5,1.5,3.5), float4(2,3,2.75,4)), float4(1.0, 2.5, 2.75, 4.0));
	test.expectEq(math.clamp(a, 1.5, 3.0), float4(1.5, 2.0, 3.0, 3.0));
	test.expectEq(math.max(a, b), float4(3, 4, 5, 6));
	test.expectEq(math.max(a, 1.5), float4(1.5, 2, 3, 4));
	test.expectEq(math.min(a, b), float4(1, 2, 3, 4));
	test.expectEq(math.min(a, 1.5), float4(1, 1.5, 1.5, 1.5));
	test.expectEq(math.mix(a, b, 0.5), float4(2, 3, 4, 5));
	test.expectEq(math.mix(a, b, float4(0.5, 1.0, 2.0, 1.0)), float4(2, 4, 7, 6));

	test.expectTrue(math.all(a));
	test.expectTrue(math.any(a));
	test.expectFalse(math.all(c));
	test.expectTrue(math.any(c));
}

// float4 x scalar math
void test_float4_scalar_math()
{
	test.beginTest("float4_scalar_math");
	float4 a = float4(3, 4, 5, 6);
	test.expectEq(a * 0.0, float4(0));
	test.expectEq(a * 1.0, float4(3, 4, 5, 6));
	test.expectEq(a.wzyx * 0.5, float4(3.0, 2.5, 2.0, 1.5));
	
	test.expectEq(a / 2.0, float4(1.5, 2, 2.5, 3));
	test.expectEq(a + 2.0, float4(5, 6, 7, 8));
	test.expectEq(a - 2.0, float4(1, 2, 3, 4));
	test.expectEq(a * 2.0, float4(6, 8, 10, 12));
}

void main()
{
	test.beginSystem("Float4");
	{
		test_float4_swizzle();
		test_float4_float4_math();
		test_float4_scalar_math();
	}
	test.report();
}
