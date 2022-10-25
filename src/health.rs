use actix_web::{web, HttpResponse};
use diesel::dsl::sql;
use diesel::sql_types::Bool;
use diesel::RunQueryDsl;
use poac_api_utils::DbPool;

pub(crate) async fn db(pool: web::Data<DbPool>) -> HttpResponse {
    let is_database_connected = web::block(move || {
        if let Ok(mut conn) = pool.get() {
            sql::<Bool>("SELECT 1").execute(&mut conn).is_ok()
        } else {
            false
        }
    })
    .await
    .unwrap_or(false);

    let db_status = serde_json::json!({ "database_connected": is_database_connected }).to_string();
    if is_database_connected {
        HttpResponse::Ok()
            .content_type("application/json")
            .body(db_status)
    } else {
        HttpResponse::ServiceUnavailable()
            .content_type("application/json")
            .body(db_status)
    }
}

pub(crate) async fn api() -> HttpResponse {
    HttpResponse::Ok()
        .content_type("application/json")
        .body("ok")
}
