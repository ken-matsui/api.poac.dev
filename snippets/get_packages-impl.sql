select * from packages
order by name, string_to_array(version, '.')::int[] desc;

