#include <err.h>
#include <gcodes.h>
#include <stdio.h>

static int islast(unsigned char c)
{
    return c == 0 || c == ';' || c == '\n';
}

static int read_int(const unsigned char **str, const unsigned char *end, int32_t *val)
{
    if (*str >= end)
        return -E_BADNUM;
    if ((**str >= '0' && **str <= '9') || **str == '-') {
        int32_t v = 0;
        int8_t minus = 1;
        if (**str == '-') {
            minus = -1;
            (*str)++;
        }
        if (*str >= end)
            return -E_BADNUM;
        while (**str >= '0' && **str <= '9' && *str < end) {
            v *= 10;
            v += (**str - '0');
            (*str)++;
        }
        v *= minus;
        *val = v;
        return E_OK;
    }
    return -E_BADNUM;
}

static uint8_t hex_decode(unsigned char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return 0xFF;
}

static uint8_t is_hex(unsigned char c)
{
    if (c >= '0' && c <= '9')
        return 1;
    if (c >= 'a' && c <= 'f')
        return 1;
    if (c >= 'A' && c <= 'F')
        return 1;
    return 0;
}

static int read_hex(const unsigned char **str, const unsigned char *end, int32_t *val)
{
    if (*str >= end)
        return -E_BADNUM;
    if (is_hex(**str)) {
        int32_t v = 0;
        while (is_hex(**str) && *str < end) {
            v *= 16;
            v += hex_decode(**str);
            (*str)++;
        }
        *val = v;
        return E_OK;
    }
    return -E_BADNUM;
}

static int read_double(const unsigned char **str, const unsigned char *end, double *val)
{
    if (*str >= end)
        return -E_BADNUM;

    if ((**str >= '0' && **str <= '9') || **str == '-' || **str == '.')
    {
        int8_t minus = 1;
        double v = 0;

        if (**str == '-')
        {
            minus = -1;
            (*str)++;
        }

        if (*str >= end)
            return -E_BADNUM;

        while (**str >= '0' && **str <= '9' && *str < end)
        {
            v *= 10;
            v += (**str - '0');
            (*str)++;
        }

        if (**str == '.')
        {
            uint8_t s = 0;
            double div = 10;
            (*str)++;
            while (**str >= '0' && **str <= '9' && *str < end)
            {
                v += (**str - '0') / div;
                (*str)++;
                div *= 10;
            }
        }

        v *= minus;
        *val = v;
        return E_OK;
    }
    return -E_BADNUM;
}

static int parse_element(const unsigned char **str, const unsigned char *end, gcode_cmd_t *cmd)
{
    if (str == NULL || *str == NULL || *str >= end)
        return -E_NULL;

    while (**str == ' ')
        (*str)++;

    if (islast(**str) || *str >= end) {
        cmd->type = 0;
        return E_OK;
    }

    if (!((**str >= 'A' && **str <= 'Z') || **str == '*'))
        return -E_INCORRECT;

    switch ((*str)[0]) {
    default:
        cmd->type = **str;
        (*str)++;
        if (*str < end)
        {
            if (read_int(str, end, &(cmd->val_i)))
                return -E_BADNUM;
        }
        else
        {
            return -E_BADNUM;
        }
        break;
    case 'A':
    case 'B':
    case 'P':
    case 'L':
    case 'F':
    case 'T':
        cmd->type = **str;
        (*str)++;
        if (*str < end)
        {
            if (**str == ' ' || **str == '\n' || **str == 0) {
                return -E_BADNUM;
            }
            if (read_double(str, end, &(cmd->val_f)))
                return -E_BADNUM;
        }
        else
        {
            return -E_BADNUM;
        }
        break;
    case '*':
        cmd->type = '*';
        (*str)++;
        if (read_hex(str, end, &(cmd->val_i)))
            return -E_BADNUM;

        break;
    }

    return E_OK;
}

int parse_cmdline(const unsigned char *str, size_t len, gcode_frame_t *frame)
{
    int i = 0, rc;
    const unsigned char *str0 = str, *end = str + len;
    int finish = 0;
    while (*str != 0 && i < MAX_CMDS && !finish && str < end)
    {
        if ((rc = parse_element(&str, end, &(frame->cmds[i]))) < 0)
        {
            return rc;
        }
        switch (frame->cmds[i].type)
        {
            case '*':
            {
                int sum = 0;
                int crc = frame->cmds[i].val_i;
                while (*str0 != '*')
                {
                    sum += (*str0);
                    str0++;
                }
                if (sum != crc)
                {
                    return -E_CRC;
                }
                finish = 1;
                break;
            }
            case 0:
            {
                finish = 1;
                break;
            }
            default:
            {
                i++;
                break;
            }
        }
    }
    frame->num  = i;
    return -E_OK;
}
