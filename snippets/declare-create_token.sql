create or replace function create_token(name text, token text)
returns record
language plpgsql
as $$
declare
  id_v uuid = uuid_generate_v4();
  created_at_v timestamp := now();
begin 
  insert into public.tokens (id, created_at, name, token)
  values (id_v, created_at_v, name, crypt(token, gen_salt('bf')));
  return (id_v, created_at_v);
end
$$;

