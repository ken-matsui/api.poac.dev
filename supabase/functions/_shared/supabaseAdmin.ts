import { createClient } from "https://esm.sh/@supabase/supabase-js@^1.33.2";

export const supabaseAdmin = createClient(
  // Supabase API URL - env var exported by default.
  Deno.env.get("SUPABASE_URL") ?? "",
  // Supabase API SERVICE ROLE KEY - env var exported by default.
  // WARNING: The service role key has admin priviliges and should only be used in secure server environments!
  Deno.env.get("SUPABASE_SERVICE_ROLE_KEY") ?? "",
);
