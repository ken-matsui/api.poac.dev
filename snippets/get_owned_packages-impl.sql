select distinct on (p.name) * from packages p
left join ownerships o on o.package_name = p.name
left join users u on u.id = o.user_id
where u.user_name = 'ken-matsui'
order by p.name, string_to_array(p.version, '.')::int[] desc;

