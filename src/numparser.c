Error numparser_cstr_to_lld(const char* str, lld_t* out_lld)
{
    lld_t ret_lld = 0;
    lld_t sign    = 1;
    if (str == NULL)
    {
        LOG_ERROR("Null pointer found");
        return ERR_NULL;
    }
    if (str[0] == '-')
    {
        sign = -1;
        str++;
    }
    else if (str[0] == '+')
    {
        str++;
    }
    if (str[0] == '\0')
    {
        LOG_ERROR("Cannot convert the provided string into an integer");
        return ERR_PARSE_STRING_TO_LLD;
    }
    while (*str != '\0')
    {
        if ((*str < '0') || (*str > '9'))
        {
            LOG_ERROR("Cannot convert string containing `%c`", *str);
            return ERR_PARSE_STRING_TO_LLD;
        }
        ret_lld = ret_lld * 10 + (*str - '0');
        str++;
    }
    *out_lld = ret_lld * sign;
    return ERR_ALL_GOOD;
}

Error numparser_cstr_to_llu(const char* str, llu_t* out_llu_t_p)
{
    llu_t ret_uint = 0;
    if (str == NULL)
    {
        LOG_ERROR("Null pointer found");
        return ERR_NULL;
    }
    if (str[0] == '+')
    {
        str++;
    }
    if (str[0] == '\0')
    {

        LOG_ERROR("Cannot convert the provided string into a size_t");
        return ERR_PARSE_STRING_TO_LLU;
    }
    while (*str != '\0')
    {
        if ((*str < '0') || (*str > '9'))
        {
            LOG_ERROR("Cannot convert string containing `%c`", *str);
            return ERR_PARSE_STRING_TO_LLU;
        }
        ret_uint = ret_uint * 10 + (*str - '0');
        str++;
    }
    *out_llu_t_p = ret_uint;
    return ERR_ALL_GOOD;
}

Error numparser_cstr_to_double(const char* str, double* out_double)
{
    llu_t ret_double = 0;
    bool dot_found   = false;
    int divider      = 1;
    if (str == NULL)
    {
        LOG_ERROR("Null pointer found");
        return ERR_NULL;
    }
    if (str[0] == '-')
    {
        divider = -1;
        str++;
    }
    else if (str[0] == '+')
    {
        str++;
    }
    if (str[0] == '\0')
    {
        LOG_ERROR("Cannot convert the provided string into an double");
        return ERR_PARSE_STRING_TO_DOUBLE;
    }
    while (*str != '\0')
    {
        if (*str == '.')
        {
            if (dot_found)
            {
                // This is the second dot found. Throw an error.
                LOG_ERROR("Invalid string: too many decimal separators found");
                return ERR_PARSE_STRING_TO_DOUBLE;
            }
            else
            {
                dot_found = true;
                str++;
                continue;
            }
        }
        if ((*str < '0') || (*str > '9'))
        {
            LOG_ERROR("Cannot convert string containing `%c`", *str);
            return ERR_PARSE_STRING_TO_DOUBLE;
        }

        ret_double = ret_double * 10LLU + (llu_t)(*str - '0');

        if (dot_found)
        {
            divider *= 10;
        }
        str++;
    }
    *out_double = (double)ret_double / (double)divider;
    return ERR_ALL_GOOD;
}

double numparser_rounder(double to_be_rounded, double step, llu_t num_of_decimals)
{
    lld_t to_be_rounded_int;
    lld_t step_int;
    for (size_t i = 0; i < num_of_decimals; i++)
    {
        to_be_rounded *= 10.0f;
        step *= 10.0f;
    }
    to_be_rounded_int = (lld_t)to_be_rounded;
    step_int          = (lld_t)step;

    lld_t remainder = to_be_rounded_int % step_int;
    lld_t adjuster;
    if (to_be_rounded > 0)
    {
        adjuster = (double)remainder / step > 0.5 ? step_int : (lld_t)0;
    }
    else
    {
        adjuster = (double)remainder / step < -0.5 ? -step_int : (lld_t)0;
    }
    lld_t quotient_int     = (to_be_rounded_int / step_int) * step_int + adjuster;
    double quotient_double = (double)quotient_int;
    for (size_t i = 0; i < num_of_decimals; i++)
    {
        quotient_double /= 10.0f;
    }
    return quotient_double;
}

#ifdef _TEST
void test_numparser(void)
{
    PRINT_BANNER();

    PRINT_TEST_TITLE("Valid to-lld_t conversions");
    {
        lld_t parsed_lld;
        numparser_cstr_to_lld("12345", &parsed_lld);
        ASSERT_EQ(parsed_lld, 12345, "Integer successfully converted");
        numparser_cstr_to_lld("0", &parsed_lld);
        ASSERT_EQ(parsed_lld, 0, "Zero successfully converted");
        numparser_cstr_to_lld("-0", &parsed_lld);
        ASSERT_EQ(parsed_lld, 0, "Negative zero successfully converted");
        numparser_cstr_to_lld("-1", &parsed_lld);
        ASSERT_EQ(parsed_lld, -1, "Negative integer successfully converted");
        numparser_cstr_to_lld("+1", &parsed_lld);
        ASSERT_EQ(parsed_lld, +1, "Positive integer with sign successfully converted");
        numparser_cstr_to_lld("0002", &parsed_lld);
        ASSERT_EQ(parsed_lld, 2, "Positive integer with leading zeros converted");
    }
    PRINT_TEST_TITLE("Invalid to-lld_t conversions");
    {
        lld_t parsed_lld;
        ASSERT(
            numparser_cstr_to_lld("1f5", &parsed_lld) == ERR_PARSE_STRING_TO_LLD, "Invalid string detected");
        ASSERT(
            numparser_cstr_to_lld("1f5", &parsed_lld) == ERR_PARSE_STRING_TO_LLD, "Invalid string detected");
        ASSERT(
            numparser_cstr_to_lld("+-05", &parsed_lld) == ERR_PARSE_STRING_TO_LLD, "Invalid string detected");
        ASSERT(numparser_cstr_to_lld(NULL, &parsed_lld) == ERR_NULL, "Null string detected");
        ASSERT(numparser_cstr_to_lld("", &parsed_lld) == ERR_PARSE_STRING_TO_LLD, "Empty string detected");
    }
    PRINT_TEST_TITLE("Valid to-size_t conversions");
    {
        llu_t parsed;
        numparser_cstr_to_llu("1234567890123", &parsed);
        ASSERT_EQ(parsed, 1234567890123, "size_t successfully converted");
        numparser_cstr_to_llu("0", &parsed);
        ASSERT_EQ(parsed, 0, "Zero successfully converted");
        numparser_cstr_to_llu("+1", &parsed);
        ASSERT_EQ(parsed, +1, "size_t with sign successfully converted");
        numparser_cstr_to_llu("002", &parsed);
        ASSERT_EQ(parsed, 2, "size_t with leading zeros converted");
    }
    PRINT_TEST_TITLE("Invalid to-size_t conversions");
    {
        llu_t parsed;
        ASSERT(numparser_cstr_to_llu("1f5", &parsed) == ERR_PARSE_STRING_TO_LLU, "Invalid string detected");
        ASSERT(numparser_cstr_to_llu("-235", &parsed) == ERR_PARSE_STRING_TO_LLU, "Invalid string detected");
        ASSERT(numparser_cstr_to_llu("+-05", &parsed) == ERR_PARSE_STRING_TO_LLU, "Invalid string detected");
        ASSERT(numparser_cstr_to_llu(NULL, &parsed) == ERR_NULL, "Null string detected");
        ASSERT(numparser_cstr_to_llu("", &parsed) == ERR_PARSE_STRING_TO_LLU, "Empty string detected");
    }
    PRINT_TEST_TITLE("Valid to-double conversions");
    {
        double parsed_double;
        numparser_cstr_to_double("12.345", &parsed_double);
        ASSERT_EQ(12.345, parsed_double, "Double successfully converted");
        numparser_cstr_to_double("-12.345", &parsed_double);
        ASSERT_EQ(parsed_double, -12.345, "Negative double successfully converted ");
        numparser_cstr_to_double("1.4", &parsed_double);
        ASSERT_EQ(parsed_double, 1.4, "Double successfully converted ");
        ASSERT(numparser_cstr_to_double("-12", &parsed_double) == ERR_ALL_GOOD, "Ok");
        ASSERT_EQ(parsed_double, -12.0, " Negative double with no decimals successfully converted ");
        numparser_cstr_to_double("12", &parsed_double);
        ASSERT_EQ(parsed_double, 12.0, " Positive double with no decimals successfully converted ");
        numparser_cstr_to_double("+12", &parsed_double);
        ASSERT_EQ(parsed_double, 12.0, "Positive double with explicit sign and no decimals successfully converted");
    }
    PRINT_TEST_TITLE("Invalid to-double conversions");
    {
        double parsed_double;
        ASSERT(numparser_cstr_to_double("1.5.4", &parsed_double) == ERR_PARSE_STRING_TO_DOUBLE, "Too many dots detected.");
        ASSERT(numparser_cstr_to_double("1r5r4", &parsed_double) == ERR_PARSE_STRING_TO_DOUBLE, "Invalid string detected");
    }
    PRINT_TEST_TITLE("Rounding");
    {
        ASSERT_EQ(numparser_rounder(12.34, 0.1, 2), 12.30, "Rounding correct.");
        ASSERT_EQ(numparser_rounder(12.34, 1.0, 2), 12.00, "Rounding correct.");
        ASSERT_EQ(numparser_rounder(12.34, 0.2, 2), 12.40, "Rounding UP correct.");
        ASSERT_EQ(numparser_rounder(-12.34, 0.1, 2), -12.30, "Rounding correct.");
        ASSERT_EQ(numparser_rounder(-12.34, 1.0, 2), -12.00, "Rounding correct.");
        ASSERT_EQ(numparser_rounder(-12.34, 0.2, 2), -12.40, "Rounding UP correct.");
    }
    /**/
}
#endif /* _TEST */
