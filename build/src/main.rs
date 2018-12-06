extern crate handlebars;
#[macro_use] extern crate serde_json;
#[macro_use] extern crate serde_derive;
#[macro_use] extern crate lazy_static;
#[macro_use] extern crate structopt;
extern crate toml;
extern crate regex;

use std::error::Error;
use std::fs::{self, File};
use std::io::{Read, BufRead, BufReader};
use std::path::{Path, PathBuf};
use std::sync::Mutex;

use structopt::StructOpt;
use regex::Regex;
use handlebars::{
    Context, Handlebars, Helper, Output, RenderContext, RenderError,
};

#[derive(Deserialize, Debug)]
struct Config {
    template: String,
    output: String,
    include_path: Vec<String>,
    source_path: Vec<String>,
    ignore_file: Vec<String>,
}

#[derive(Debug, Serialize, Deserialize)]
struct Project {
    include_files: Vec<PathBuf>,
    source_files: Vec<PathBuf>,
}

#[derive(StructOpt, Debug)]
#[structopt(name = "gen")]
struct Options {
    /// Project toml file to use
    #[structopt(short = "i", long = "input", parse(from_os_str))]
    input: PathBuf,
}

lazy_static! {
    static ref prj : Mutex<Project> = Mutex::new(Project {
        include_files: vec![],
        source_files: vec![],
    });
}

fn include(
    h: &Helper,
    _: &Handlebars,
    _: &Context,
    _: &mut RenderContext,
    out: &mut Output,
) -> Result<(), RenderError> {
    let param = h
        .param(0)
        .ok_or(RenderError::new("Param 0 is required for format helper."))?;
    let file = param.value().as_str();
    out.write(&*format!("// file: {}\n", file.unwrap()))?;
    let text = load_clean_contents(&*file.unwrap());
    out.write(text.as_ref())?;
    Ok(())
}

fn include_raw(
    h: &Helper,
    _: &Handlebars,
    _: &Context,
    _: &mut RenderContext,
    out: &mut Output,
) -> Result<(), RenderError> {
    let param = h
        .param(0)
        .ok_or(RenderError::new("Param 0 is required for format helper."))?;
    let file = param.value().as_str();
    out.write(&*format!("// file: {}\n", file.unwrap()))?;
    let text = load_raw_contents(&*file.unwrap());
    out.write(text.as_ref())?;
    Ok(())
}

fn header_files(
    _: &Helper,
    _: &Handlebars,
    _: &Context,
    _: &mut RenderContext,
    out: &mut Output,
) -> Result<(), RenderError> {
    let list = &prj.lock().unwrap().include_files.clone();
    for it in list {
        let file = it.to_string_lossy();
        out.write(&*format!("// file: {}\n", file))?;
        let text = load_clean_contents(&*file);
        out.write(text.as_ref())?;
    }
    Ok(())
}

fn source_files(
    _: &Helper,
    _: &Handlebars,
    _: &Context,
    _: &mut RenderContext,
    out: &mut Output,
) -> Result<(), RenderError> {
    let list = &prj.lock().unwrap().source_files.clone();
    for it in list {
        let file = it.to_string_lossy();
        out.write(&*format!("// file: {}\n", file))?;
        let text = load_clean_contents(&*file);
        out.write(text.as_ref())?;
    }
    Ok(())
}

fn include_and_expand(
    h: &Helper,
    _: &Handlebars,
    _: &Context,
    _: &mut RenderContext,
    out: &mut Output,
) -> Result<(), RenderError> {
    let param = h
        .param(0)
        .ok_or(RenderError::new("Param 0 is required for format helper."))?;
    let file = param.value().as_str();
    let text = expand_include_contents(file.unwrap());
    out.write(text.as_ref())?;
    Ok(())
}

fn should_use_file(file: &Path, ingore_list: &[String]) -> bool
{
    let lossy_name = file.to_string_lossy();
    for file in ingore_list {
        if lossy_name.ends_with(file) {
            return false;
        }
    }
    true
}

fn find_file(file: &str, file_list: &[PathBuf]) -> Option<PathBuf> {
    for name in file_list {
        if name.ends_with(file) {
            return Some(name.clone());
        }
    }
    None
}

fn consume_file(file: &str, list: &mut Vec<PathBuf>) {
    println!(">> {}", file);
    let index = list.iter().position(|x| x.ends_with(file)).unwrap();
    list.remove(index);
}

fn expand_include_contents(file: &str) -> String {
    let list = &prj.lock().unwrap().include_files.clone();
    let fullname = match find_file(file, &list) {
        Some(n) => {
            consume_file(file, &mut prj.lock().unwrap().include_files);
            n
        },
        None => {
            return format!("// ERROR: <{} not found>", file);
        }
    };

    let f = match File::open(fullname) {
        Ok(f) => f,
        Err(_) => return format!("// ERROR: could not open: {}", file)
    };

    let re = Regex::new(r#"^\s*#\s*include\s+"(?P<include>.*)""#).unwrap();

    let mut contents = String::new();
    let buf = BufReader::new(f);
    for line in buf.lines() {
        let l = line.unwrap();
        if re.is_match(&*l) {
            let r = re.replace(&*l, "// including file: $include\n");
            let hdr = re.replace(&*l, "$include");
            contents.push_str(r.as_ref());
            let expand = load_clean_contents(&*hdr);
            contents.push_str(format!("// BEGIN file: {}\n", l).as_ref());
            contents.push_str(&*expand);
            contents.push('\n');
            contents.push_str(format!("// END file: {}\n", l).as_ref());
        } else {
            contents.push_str(&*l);
        }
        contents.push('\n');
    }

    contents
}

fn load_raw_contents(file: &str) -> String {
    load_contents(file, false)
}

fn load_clean_contents(file: &str) -> String {
    load_contents(file, true)
}

fn load_contents(file: &str, clean: bool) -> String {
    let list = prj.lock().unwrap().include_files.clone();
    let fullname = match find_file(file, &list) {
        Some(n) => {
            consume_file(file, &mut prj.lock().unwrap().include_files);
            n
        },
        None => {
            let list = prj.lock().unwrap().source_files.clone();
            match find_file(file, &list) {
                Some(n) => {
                    consume_file(file, &mut prj.lock().unwrap().source_files);
                    n
                },
                None => {
                    return format!("// ERROR: <{} not found>", file);
                }
            }
        }
    };

    let f = match File::open(&*fullname) {
        Ok(f) => f,
        Err(_) => return format!("// ERROR: could not open: {}", file)
    };

    let re = Regex::new(r#"^\s*#\s*include\s+"(?P<include>.*)""#).unwrap();
    let mut contents = String::new();
    let buf = BufReader::new(f);
    for line in buf.lines() {
        let l = line.unwrap();
        if clean && re.is_match(&*l) {
            contents.push_str(&*format!("// {}", &*l));
        } else {
            contents.push_str(&*l);
        }
        contents.push('\n');
    }
    contents
}

fn collect_files(dir: &str, ignore_list: &[String]) -> Result<Vec<PathBuf>, Box<Error>> {
    let mut v = vec![];
    let dir = Path::new(dir);
    if dir.is_file() && should_use_file(&dir, &ignore_list) {
        v.push(PathBuf::from(dir));
    }
    else {
        for entry in dir.read_dir()? {
            let entry = entry?;
            let path = entry.path();
            let metadata = fs::metadata(&path)?;
            if metadata.is_file() && should_use_file(&path, &ignore_list) {
                v.push(path);
            }
        }
    }
    Ok(v)
}

fn main() -> Result<(), Box<Error>> {
    let opt = Options::from_args();
    let mut cfg_file = File::open(opt.input)?;
    let mut contents = String::new();
    cfg_file.read_to_string(&mut contents)?;
    let cfg: Config = toml::from_str(&*contents)?;
    //println!("{:#?}", cfg);

    for path in cfg.include_path {
        println!("loading: {}", path);
        prj.lock().unwrap().include_files.append(&mut collect_files(&*path, &cfg.ignore_file)?);
    }
    for path in cfg.source_path {
        println!("loading: {}", path);
        prj.lock().unwrap().source_files.append(&mut collect_files(&*path, &cfg.ignore_file)?);
    }
    //println!("{:#?}", prj);

    let mut tpl = Handlebars::new();
    tpl.register_helper("include_and_expand", Box::new(include_and_expand));
    tpl.register_helper("include", Box::new(include));
    tpl.register_helper("include_raw", Box::new(include_raw));
    tpl.register_helper("header_files", Box::new(header_files));
    tpl.register_helper("source_files", Box::new(source_files));

    let data = json!({"name": ""});

    let mut source_template = File::open(cfg.template)?;
    let mut output_file = File::create(cfg.output)?;
    tpl
    .render_template_source_to_write(&mut source_template, &data, &mut output_file)?;
    println!("ASS Generated.");
    println!("Unused files:");
    println!("{:#?}", prj.lock().unwrap().include_files);
    println!("{:#?}", prj.lock().unwrap().source_files);

    Ok(())
}