use crate::auth::get_user_meta::UserMeta;
use crate::schema::users;
use crate::user::models::User;
use diesel::prelude::*;
use poac_api_utils::DbError;

#[derive(Insertable)]
#[diesel(table_name = users)]
struct NewUser<'a> {
    user_name: &'a str,
    avatar_url: &'a str,
    name: &'a str,
    email: &'a str,
    status: &'a str,
}

pub(crate) fn create_user(
    conn: &mut PgConnection,
    user_meta: &UserMeta,
    email: &str,
) -> Result<User, DbError> {
    let new_user = NewUser {
        user_name: &user_meta.user_name,
        avatar_url: &user_meta.avatar_url,
        name: &user_meta.name,
        email,
        status: "active",
    };

    let result = diesel::insert_into(users::table)
        .values(&new_user)
        .returning((users::id, users::name, users::user_name, users::avatar_url))
        .get_result::<User>(conn)?;

    Ok(result)
}
