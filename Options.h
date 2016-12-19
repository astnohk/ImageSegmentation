
struct OPTIONS
{
	SIZE ResampleSize;
	unsigned long ResampleMethod;
	unsigned long Mode;
	unsigned long Output;

	OPTIONS(void);
	bool ChangeResampleMethod(const char *name);
	void set_default(const char *name);
	void set_value(const char *name, const void *value);
};

// Mode Options
#define MODE_MEAN_SHIFT 0x0001

// Output Options
#define OUTPUT_NEGATE 0x01
#define OUTPUT_RESAMPLED 0x02

#endif
