use crate::user::models::User;
use actix_web::error::ErrorInternalServerError;
use actix_web::web::ServiceConfig;
use actix_web::{get, web, HttpResponse, Result};
use diesel::prelude::*;
use diesel::PgConnection;
use poac_api_utils::{concat_org_name, DbPool, Response};
use poac_api_utils::{log_query, DbError};

fn get_owners(conn: &mut PgConnection, name_: &str) -> Result<Vec<User>, DbError> {
    use crate::schema::ownerships::dsl::{ownerships, package_name, user_id};
    use crate::schema::users::dsl::*;

    let query = users
        .select((id, name, user_name, avatar_url))
        .left_join(ownerships.on(package_name.eq(name_)))
        .filter(user_id.eq(id))
        .filter(status.eq("active"));
    log_query(&query);

    let results = query.load::<User>(conn)?;
    Ok(results)
}

async fn owners_impl(pool: web::Data<DbPool>, name: String) -> Result<HttpResponse> {
    let users = web::block(move || {
        let mut conn = pool.get()?;
        get_owners(&mut conn, &name)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    Ok(Response::ok(users))
}

#[get("/packages/{org}/{name}/owners")]
async fn owners(
    pool: web::Data<DbPool>,
    param: web::Path<(String, String)>,
) -> Result<HttpResponse> {
    let (org, name) = param.into_inner();
    owners_impl(pool, concat_org_name(org, name)).await
}

#[get("/packages/{name}/owners")]
async fn owners_official(pool: web::Data<DbPool>, name: web::Path<String>) -> Result<HttpResponse> {
    owners_impl(pool, name.into_inner()).await
}

pub(crate) fn init_routes(cfg: &mut ServiceConfig) {
    cfg.service(owners);
    cfg.service(owners_official);
}
