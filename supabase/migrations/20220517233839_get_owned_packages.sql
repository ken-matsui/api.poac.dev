create function get_owned_packages(username text)
returns table (
    id uuid,
    published_at timestamp,
    metadata jsonb,
    name text,
    version text,
    edition int2,
    authors text[],
    repository text,
    description text,
    license text,
    readme text,
    sha256sum varchar(64)
)
language sql
as $$
    select
        distinct on (p.name)
        p.id,
        p.published_at,
        p.metadata,
        p.name,
        p.version,
        p.edition,
        p.authors,
        p.repository,
        p.description,
        p.license,
        p.readme,
        p.sha256sum
    from packages p
    left join ownerships o on o.package_name = p.name
    left join users u on u.id = o.user_id
    where u.user_name = username
    order by p.name, string_to_array(p.version, '.')::int[] desc;
$$;
