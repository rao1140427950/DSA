#include <iostream>
#include <fstream>
#include <gmp.h>
#include <cmath>
#include <bitset>
#include <cstdlib>
#include <cstring> 
#include <sstream>
#include <ctime>
//using namespace std;
//#macros
const int L = 1024;
const int N = 160;

#define P "p=102865584259843077175583195011997798900482038016705824136288380475734860009055428071534495956844807748416572686838253895244634687898659646424515259679129905513743899853971066468883670407530107234961085482225328667572772611162756643027105617873895021996158552984843708233824989792811721408577351617080369547993"
#define Q "q=734415599462729831694143846331445277609193755927"
#define G "g=63615006880335642768473038477258757436464860136916565207798584167060621564899979263408565137993978149206751054438974059615983337126379668370747907507911540381031959187353048278562320341063050939775344313271013777131358834376209974551749493023310606751625276738876397935042130121966817767949476523717161640453"
#define X "x=339119201894965867922409227633199021527378715543"
#define Y "y=1099906791313925528746008054081768734007884349815325963667520491768596235922636596649198172987598573083011790017146356061273962023338014420645127092468263770753970716461208880423045761205934804880887634821616587683235765408867072852094816664326084550730344050243082288308837441908172297994552279650972016922"
#define MESSAGE "m=16711083"


typedef struct {
	mpz_t p;
	mpz_t q;
	mpz_t g;
} tuple;

typedef struct {
	bool valid;
	mpz_t p;

}m_inverse;

typedef struct {
	mpz_t x;
	mpz_t y;
} key_pair;

typedef struct {
	mpz_t r;
	mpz_t s;
} sign_pair;

typedef struct {
	std::string  M;
}message_digest;


key_pair genkey_pair(tuple* pqg) { // generates a new ``key_pair'' with a pseudo random x 
	key_pair pair;
	unsigned long x_raise = rand();

	mpz_t x;
	mpz_t y;

	mpz_init(pair.x);
	mpz_init(pair.y);
	mpz_init(y);

	mpz_init_set_ui(x, x_raise);
	mpz_powm_ui(x, x, x_raise, (*pqg).q); // big random number is create with boundarys 0 < x < q


	do { // while x is less then p find next prime for x
		mpz_nextprime(x, x);

	} while (!(mpz_cmp(x, (*pqg).p) < 0));
	mpz_set(pair.x, x);

	// y is created from raising g to x modulo p
	mpz_powm(y, (*pqg).g, x, (*pqg).p);
	mpz_set(pair.y, y);

	mpz_clear(x);
	mpz_clear(y);

	return	pair;
}


//C.1 Computation of the Inverse Value;
m_inverse inverse_value(mpz_t* z, mpz_t* a) { //from assignment 2
	mpz_t i, j, y, y1, y2, rem, quotient, tmp;
	m_inverse zinv;
	mpz_init(zinv.p);
	//commits is fun!

	if (!(mpz_cmp(*a, *z) && mpz_cmp_si(*z, 0))) {
		std::cout << "invalid arguments" << std::endl;
		zinv.valid = 0;
		return zinv;
	}
	mpz_init(i); mpz_init(j); mpz_init(y); mpz_init(y1);
	mpz_init(y2); mpz_init(rem); mpz_init(quotient); mpz_init(tmp);
	mpz_set(i, *a);
	mpz_set(j, *z);
	mpz_set_ui(y2, 0);
	mpz_set_ui(y1, 1);
	do {
		//quotient = floor((i+0.0)/(j+0.0));
		mpz_div(quotient, i, j);

		//rem = i - ( j * quotient);
		mpz_mul(tmp, j, quotient);
		mpz_sub(rem, i, tmp);

		//y = y2 - (y1 * quotient);
		mpz_mul(tmp, y1, quotient);
		mpz_sub(y, y2, tmp);

		//i = j; j = rem; y2 = y1;y1 = y;
		mpz_set(i, j);
		mpz_set(j, rem);
		mpz_set(y2, y1);
		mpz_set(y1, y);

	} while (mpz_cmp_si(j, 0));
	if (mpz_cmp_si(i, 1) != 0) {
		std::cout << "Error_" << std::endl;
		zinv.valid = 0;
		return zinv;
	};

	zinv.valid = 1;
	//zinv.p = (y2 % a);
	mpz_mod(zinv.p, y2, *a);
	mpz_clear(i); mpz_clear(j); mpz_clear(y); mpz_clear(y1);
	mpz_clear(y2); mpz_clear(rem); mpz_clear(quotient); mpz_clear(tmp);
	return zinv;
}

sign_pair signing_operation(tuple* pqg, key_pair(*a), message_digest* m) {
	mpz_t r, s, k, k_inv, z, tmp;
	key_pair kp = genkey_pair(pqg);
	sign_pair out;
	mpz_init(out.r);
	mpz_init(out.s);
	mpz_init(r); mpz_init(s); mpz_init(k); mpz_init(k_inv); mpz_init(tmp); mpz_init(z);
	mpz_set(k, kp.x);


	std::string message_ = ((*m).M);
	mpz_set(k_inv, inverse_value(&k, &(*pqg).q).p);

	mpz_powm(r, (*pqg).g, k, (*pqg).p);
	mpz_mod(r, r, (*pqg).q);

	//z = the leftmost min(N,outlen) bits of hash(M) 
	mpz_set_str(z, message_.c_str(), 16);

	//s = (k^-1)(z+xr) mod q
	mpz_mul(tmp, (*a).x, r);
	mpz_add(tmp, tmp, z);


	mpz_mul(s, k_inv, tmp);
	mpz_mod(s, s, (*pqg).q);
	mpz_set(out.r, r);
	mpz_set(out.s, s);
	// 

	mpz_clear(r); mpz_clear(s); mpz_clear(k); mpz_clear(k_inv); mpz_clear(tmp); mpz_clear(z);
	return out;
}

//Defines the signing operation
bool verification_algorithm(tuple* pqg, mpz_t* y, message_digest* m, sign_pair* rs) {
	mpz_t w, u1, u2, v, tmp2, tmp1, z;

	mpz_init(w); mpz_init(u1); mpz_init(u2); mpz_init(v); mpz_init(tmp1); mpz_init(tmp2);
	mpz_init(z);
	std::string message_ = ((*m).M);

	//1;
	//check 0 < r' < q and 0 < s' < q; if either condition is violated,
	if (!((mpz_cmp_ui((*rs).r, 0) > 0) &&
		(mpz_cmp((*pqg).q, (*rs).r) > 0) && (mpz_cmp_ui((*rs).s, 0) > 0) && (mpz_cmp((*pqg).q, (*rs).s) > 0))) {
		return 0;
	}

	//2;
	// w = s'^-1 mod q
	mpz_mod(w, inverse_value(&(*rs).s, &(*pqg).q).p, (*pqg).q);

	// z = the leftmost min(N,outlen) bits of Hash(M')(should bee converted to integer)
	mpz_set_str(z, message_.c_str(), 16);
	// u1 = (zw) mod q
	mpz_mul(u1, w, z);
	mpz_mod(u1, u1, (*pqg).q);
	// u2 = ((r')w) mod q.
	mpz_mul(u2, (*rs).r, w);
	mpz_mod(u2, u2, (*pqg).q);

	// v  = (g^u1 y^u2 mod p)mod q
	mpz_powm(tmp1, (*pqg).g, u1, (*pqg).p);
	mpz_powm(tmp2, (*y), u2, (*pqg).p);
	mpz_mul(v, tmp1, tmp2);
	mpz_mod(v, v, (*pqg).p);
	mpz_mod(v, v, (*pqg).q);

	//3;
	//if v = r', then signarture is verified else return invalid 
	gmp_printf("v=%Zd\n", v);
	if (mpz_cmp(v, (*rs).r) != 0) {
		return 0;
	}

	return 1;
}


//Define the verification algorithm
int isvalid(tuple* pqg) {

	//retriving binary length's 
	int length_p = ((std::string)mpz_get_str(NULL, 2, (*pqg).p)).length();
	int length_q = ((std::string)mpz_get_str(NULL, 2, (*pqg).q)).length();

	mpz_t t, tmp1, tmp2;
	mpz_init(tmp1);
	mpz_init(tmp2);
	mpz_init(t);

	//precalculating q-1
	mpz_sub_ui(t, (*pqg).p, 1);

	//precalculating g^q mod p
	mpz_powm(tmp1, (*pqg).g, (*pqg).q, (*pqg).p);

	if (
		//both p and q are probably primes;
		mpz_probab_prime_p((*pqg).p, 25) &&
		mpz_probab_prime_p((*pqg).q, 25) &&
		//q is a divisor of p-1;
		mpz_divisible_p(t, (*pqg).q) &&
		//g has order q i.e. g^q mod p = 1 and g > 1.
		!mpz_cmp_ui(tmp1, 1) &&
		mpz_cmp_ui((*pqg).g, 1) &&
		//p is a 1024 bit number and q a 160 bit number
		(length_p == L) &&
		(length_q == N)
		) {

		std::cout << "p,q,g are valid" << std::endl;
		return 1;
	}

	std::cout << "p,q,g are invalid" << std::endl;
	return 0;
}



int main(int argc, char* argv[]) {
	tuple pqg;
	std::string line;
	srand(time(NULL)); // Initializing the pseudo random number generator based on the time. 

	mpz_init(pqg.p);
	mpz_init(pqg.q);
	mpz_init(pqg.g);

	line = P;
	mpz_set_str(pqg.p, &line.c_str()[2], 10);
	line = Q;
	mpz_set_str(pqg.q, &line.c_str()[2], 10);
	line = G;
	mpz_set_str(pqg.g, &line.c_str()[2], 10);

	if (!isvalid(&pqg)) {
		return 0;
	}

	message_digest D;
	key_pair xy;
	mpz_init(xy.y);
	mpz_init(xy.x);
	std::cout << "sign " << MESSAGE << std::endl;

	line = X;
	mpz_set_str(xy.x, &line.c_str()[2], 10);
	line = Y;
	mpz_set_str(xy.y, &line.c_str()[2], 10);

	line = MESSAGE;

	D.M = &line.c_str()[2];
	sign_pair sign = signing_operation(&pqg, &xy, &D);
	gmp_printf("r=%Zd\n", sign.r);
	gmp_printf("s=%Zd\n", sign.s);

	//key_pair xy;
	sign_pair rs;
	//message_digest D;
	mpz_init(xy.y);
	mpz_init(rs.s);
	mpz_init(rs.r);
	std::cout << "verify: " << std::endl;
	line = Y;
	mpz_set_str(xy.y, &line.c_str()[2], 10);
	mpz_set(rs.r, sign.r);
	mpz_set(rs.s, sign.s);
	if (verification_algorithm(&pqg, &xy.y, &D, &rs)) {
		std::cout << "signature is valid" << std::endl;
	}
	else {
		std::cout << "signature is invalid" << std::endl;
	}

	
	/*
	// parses data from the std input
	std::getline(std::cin, line);
	mpz_set_str(pqg.p, &line.c_str()[2], 10);
	std::getline(std::cin, line);
	mpz_set_str(pqg.q, &line.c_str()[2], 10);
	std::getline(std::cin, line);
	mpz_set_str(pqg.g, &line.c_str()[2], 10);

	if (!isvalid(&pqg)) {
		return 0;
	}
	*/

}

