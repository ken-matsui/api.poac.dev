-- https://stackoverflow.com/questions/586781/postgresql-fetch-the-rows-which-have-the-max-value-for-a-column-in-each-group
-- https://dba.stackexchange.com/questions/74283/how-to-order-by-typical-software-release-versions-like-x-y-z

select distinct on (name) * from packages
order by name, string_to_array(version, '.')::int[] desc;

