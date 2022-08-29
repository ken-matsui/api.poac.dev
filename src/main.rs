mod models;
mod schema;

use crate::models::Package;
use actix_web::error::ErrorInternalServerError;
use actix_web::{get, middleware::Logger, post, web, App, HttpResponse, HttpServer, Result};
use diesel::r2d2::{ConnectionManager, Pool};
use diesel::PgConnection;
use dotenvy::dotenv;
use serde::Deserialize;
use std::env;

type DbPool = Pool<ConnectionManager<PgConnection>>;

#[get("/v1/packages")]
async fn packages(pool: web::Data<DbPool>) -> Result<HttpResponse> {
    let other_packages = web::block(move || {
        let mut conn = pool.get()?;
        Package::find_all(&mut conn)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    Ok(HttpResponse::Ok().json(other_packages))
}

#[derive(Deserialize)]
struct Info {
    query: String,
}

#[post("/v1/search")]
async fn search(pool: web::Data<DbPool>, web::Json(body): web::Json<Info>) -> Result<HttpResponse> {
    let other_packages = web::block(move || {
        let mut conn = pool.get()?;
        Package::find(&mut conn, &body.query)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    Ok(HttpResponse::Ok().json(other_packages))
}

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    dotenv().ok();
    env_logger::init_from_env(env_logger::Env::new().default_filter_or("info"));

    // Set up database connection pool
    let database_url = env::var("DATABASE_URL").expect("DATABASE_URL must be set");
    let manager = ConnectionManager::<PgConnection>::new(database_url);
    let pool = Pool::builder()
        .build(manager)
        .expect("Failed to create pool.");

    log::info!("starting HTTP server at http://localhost:8080");

    // Start HTTP server
    HttpServer::new(move || {
        App::new()
            // Set up DB pool to be used with web::Data<Pool> extractor
            .app_data(web::Data::new(pool.clone()))
            .wrap(Logger::default())
            .service(packages)
            .service(search)
    })
    .bind(("127.0.0.1", 8080))?
    .run()
    .await
}
