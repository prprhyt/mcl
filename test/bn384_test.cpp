#define CYBOZU_TEST_DISABLE_AUTO_RUN
#include <cybozu/test.hpp>
#include <cybozu/benchmark.hpp>
#include <cybozu/option.hpp>
#include <cybozu/xorshift.hpp>
#include <mcl/bn384.hpp>
#include <mcl/bn.hpp>

using namespace mcl::bn384;

mcl::fp::Mode g_mode;

void testCurve(const mcl::bn::CurveParam& cp)
{
	initPairing(cp, g_mode);
	G1 P;
	G2 Q;
	BN::mapToG1(P, 1);
	BN::mapToG2(Q, 1);
	GT e1, e2;
	BN::pairing(e1, P, Q);
	cybozu::XorShift rg;
	mpz_class a, b;
	Fr r;
	r.setRand(rg); a = r.getMpz();
	r.setRand(rg); b = r.getMpz();
	G1 aP;
	G2 bQ;
	G1::mul(aP, P, a);
	G2::mul(bQ, Q, b);
	BN::pairing(e2, aP, bQ);
	GT::pow(e1, e1, a * b);
	CYBOZU_TEST_EQUAL(e1, e2);
	CYBOZU_BENCH_C("G1::mulCT", 500, G1::mul, aP, aP, a);
	CYBOZU_BENCH_C("G1::add", 500, G1::add, aP, aP, P);
	CYBOZU_BENCH_C("G1::dbl", 500, G1::dbl, aP, aP);
	CYBOZU_BENCH_C("G2::mulCT", 500, G2::mul, bQ, bQ, b);
	CYBOZU_BENCH_C("G2::add", 500, G2::add, bQ, bQ, Q);
	CYBOZU_BENCH_C("G2::dbl", 500, G2::dbl, bQ, bQ);
	CYBOZU_BENCH("pairing", BN::pairing, e1, P, Q);
	CYBOZU_BENCH("finalExp", BN::finalExp, e1, e1);
}

CYBOZU_TEST_AUTO(pairing)
{
	puts("CurveFp254BNb");
	// support 256-bit pairing
	testCurve(mcl::bn::CurveFp254BNb);
	puts("CurveFp382_1");
	testCurve(mcl::bn::CurveFp382_1);
	puts("CurveFp382_2");
	testCurve(mcl::bn::CurveFp382_2);
	// Q is not on EcT, but bad order
	{
		const char *s = "1 18d3d8c085a5a5e7553c3a4eb628e88b8465bf4de2612e35a0a4eb018fb0c82e9698896031e62fd7633ffd824a859474 1dc6edfcf33e29575d4791faed8e7203832217423bf7f7fbf1f6b36625b12e7132c15fbc15562ce93362a322fb83dd0d 65836963b1f7b6959030ddfa15ab38ce056097e91dedffd996c1808624fa7e2644a77be606290aa555cda8481cfb3cb 1b77b708d3d4f65aeedf54b58393463a42f0dc5856baadb5ce608036baeca398c5d9e6b169473a8838098fd72fd28b50";
		G2 Q;
		CYBOZU_TEST_EXCEPTION(Q.setStr(s, 16), std::exception);
	}
}

int main(int argc, char *argv[])
	try
{
	cybozu::Option opt;
	std::string mode;
	opt.appendOpt(&mode, "auto", "m", ": mode(gmp/gmp_mont/llvm/llvm_mont/xbyak)");
	if (!opt.parse(argc, argv)) {
		opt.usage();
		return 1;
	}
	g_mode = mcl::fp::StrToMode(mode);
	return cybozu::test::autoRun.run(argc, argv);
} catch (std::exception& e) {
	printf("ERR %s\n", e.what());
	return 1;
}
