#include "duktape.h"
#include "mbed.h"

/* Being an embeddable engine, Duktape doesn't provide I/O
 * bindings by default.  Here's a simple one argument print()
 * function.
 */

DigitalOut greenLed(LED2, 1);
InterruptIn userButton(SW0);

void blinkgreen()
{
    greenLed=0;
    Thread::wait(200);
    greenLed=1;
}

/* Adder: add argument values. */
static duk_ret_t native_blink(PinName x, int y) {
	DigitalOut dukLed(x, y);
	 if (userButton.read() == 1 ) {
		 dukLed=0;
	 }
	 else
		 dukLed=1;
	return 1;  /* one return value */
}

static duk_ret_t native_print(duk_context *ctx) {
	printf("%s\n", duk_to_string(ctx, 0));
	blinkgreen();
	return 0;  /* no return value (= undefined) */
}

/* Adder: add argument values. */
static duk_ret_t native_adder(duk_context *ctx) {
	int i;
	int n = duk_get_top(ctx);  /* #args */
	double res = 0.0;

	for (i = 0; i < n; i++) {
        res += duk_to_number(ctx, i);
    }

    duk_push_number(ctx, res);
    return 1;  /* one return value */
}

void blink_push()
{
	while(1){
		if (userButton.read() == 1 ) {
		  native_blink(LED2, 1);
		  printf("blink led\n");
		}
		else{
		  native_blink(LED2, 0);
		}
		Thread::wait(1000);
	}
}

int main(int argc, char *argv[]) {

	Thread blinky_thread;

    duk_context *ctx = duk_create_heap_default();
    const char *str;
    void *buf = duk_alloc(ctx, 1024);

    if (buf) {
       printf("allocation successful: %p\n", buf);
    } else {
      printf("allocation failed\n");
    }
    blinky_thread.start(blink_push);  // Thread to blink led on user button

    /* reading [global object].Math.PI */
    duk_push_global_object(ctx);    /* -> [ global ] */
    duk_push_string(ctx, "Math");   /* -> [ global "Math" ] */
    duk_get_prop(ctx, -2);          /* -> [ global Math ] */
    duk_push_string(ctx, "PI");     /* -> [ global Math "PI" ] */
    duk_get_prop(ctx, -2);          /* -> [ global Math PI ] */
    printf("Math.PI is %lf\n", (double) duk_get_number(ctx, -1));
    duk_pop_n(ctx, 3);

    duk_push_c_function(ctx, native_print, 1 /*nargs*/);
    duk_put_global_string(ctx, "print");
    duk_push_c_function(ctx, native_adder, DUK_VARARGS);
    duk_put_global_string(ctx, "adder");

	duk_push_number(ctx, 123);
    duk_push_string(ctx, "string_test");
    duk_push_string(ctx, "test_string2");
	
    while(1)
    {
	  str = duk_get_string(ctx, -1);
	  if (str) {
	    printf("Last pushed value is a string: %s\n", str);
	  }
	  str = duk_get_string(ctx, -2);
	  if (str) {
        printf("2nd last pushed value is a string: %s\n", str);
      }
	  Thread::wait(1000);
    }
    
    duk_destroy_heap(ctx);
    return 0;
}
