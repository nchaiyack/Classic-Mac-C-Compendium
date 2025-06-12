void Param_string( StringPtr out, StringPtr format,
	StringPtr p0, StringPtr p1, StringPtr p2 );

void Param_string( StringPtr out, StringPtr format,
	StringPtr p0, StringPtr p1, StringPtr p2 )
{
	StringPtr	out_last, format_last, out_zeroth;
	StringPtr	source, source_last;
	Boolean		saw_hat;
	
	saw_hat = false;
	out_zeroth = out;
	out_last = out + 255;
	++out;
	format_last = format + format[0];
	format++;
	
	while ((out <= out_last) && (format <= format_last))
	{
		if (saw_hat)
		{
			saw_hat = false;
			switch (*format)
			{
				case '0':	source = p0; break;
				case '1':	source = p1; break;
				case '2':	source = p2; break;
				default:	source = NIL; break;	
			}
			if (source == NIL)
			{
				*out = *format;
				++out;
			}
			else
			{
				source_last = source + source[0];
				++source;
				while ((source <= source_last) && (out <= out_last))
				{
					*out = *source;
					++out;
					++source;
				}
			}
		}
		else /* not saw_hat */
		{
			if (*format == '^')
				saw_hat = true;
			else
			{
				*out = *format;
				++out;
			}
		}
		++format;
	} /* end while */
	*out_zeroth = out + 254 - out_last;
}
