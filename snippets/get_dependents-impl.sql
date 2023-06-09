-- https://stackoverflow.com/questions/28921355/how-do-i-check-if-a-json-key-exists-in-postgres

select *
from packages
where metadata -> 'dependencies' ? 'boost/config' = true;

